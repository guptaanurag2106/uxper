package main

import (
	"encoding/binary"
	"fmt"
	"math/rand"
	"net"
	"os"
	"strings"
	"sync"
)

type DNSQuery struct {
	Domain string
	Type   uint16
	Class  uint16
}

func constructRequest(query DNSQuery) (int, []byte) {
	domain := query.Domain
	domainSplit := strings.Split(domain, ".")

	msg := make([]byte, 0, 512) // max UDP DNS packet (no EDNS)

	// header
	transactionID := rand.Intn(0x10000) // 4 bytes
	msg = binary.BigEndian.AppendUint16(msg, uint16(transactionID))

	qr := 0     // query packet
	opcode := 0 // standard query
	aa := 0     // not in request
	tc := 0     // not truncated
	rd := 1     // recursion desired
	ra := 0     // not in request
	z := 0      // reserved bit must be 0
	ad := 0     // no authenticated-data request
	cd := 0     // checking enabled
	rcode := 0  // no error for query

	flag := qr<<15 | opcode<<14 | aa<<10 | tc<<9 | rd<<8 | ra<<7 | z<<6 | ad<<5 | cd<<4 | rcode
	msg = binary.BigEndian.AppendUint16(msg, uint16(flag))

	msg = binary.BigEndian.AppendUint16(msg, 1) // qdcount
	msg = binary.BigEndian.AppendUint16(msg, 0) // ancount
	msg = binary.BigEndian.AppendUint16(msg, 0) // nscount
	msg = binary.BigEndian.AppendUint16(msg, 0) // arcount

	// question
	for i, label := range domainSplit {
		labelLen := len(label)
		if labelLen == 0 {
			if i == len(domainSplit)-1 {
				msg = append(msg, 0) // qname terminator
				continue
			} else {
				fmt.Printf("Empty label found at index %d, invalid domain name %s \n", i, domain)
				os.Exit(1)
			}
		}

		if labelLen >= 64 {
			fmt.Printf("Label %s exceeds limit of 63 bytes\n", label)
			os.Exit(1)
		}

		msg = append(msg, byte(labelLen))
		for _, c := range label {
			msg = append(msg, byte(c))
		}
	}
	msg = binary.BigEndian.AppendUint16(msg, query.Type)
	msg = binary.BigEndian.AppendUint16(msg, query.Class)

	return transactionID, msg
}

type DNSAnswer struct {
	IP      string
	CNAME   string
	Type    uint16
	Class   uint16
	TTL     uint32
	DataLen uint16
}

type Response struct {
	transactionID int
	domains       []string
	authoritative bool
	truncated     bool
	responseCode  uint8
	hasAddress    bool
	firstCNAME    string
	answers       []DNSAnswer
}

func readDNSName(response []byte, start int) (string, int) {
	idx := start
	jumped := false
	next := start
	var b strings.Builder

	for {
		labelLen := response[idx]
		if labelLen == 0 {
			if !jumped {
				next = idx + 1
			}
			break
		}

		if labelLen&0xc0 == 0xc0 {
			pointer := int(binary.BigEndian.Uint16(response[idx:idx+2]) & 0x3fff)
			if !jumped {
				next = idx + 2
			}
			idx = pointer
			jumped = true
			continue
		}

		idx++
		if b.Len() > 0 {
			b.WriteByte('.')
		}
		b.Write(response[idx : idx+int(labelLen)])
		idx += int(labelLen)
		if !jumped {
			next = idx
		}
	}

	if b.Len() > 0 {
		b.WriteByte('.')
	}

	return b.String(), next
}

func parseResponse(response []byte) Response {
	parsedResponse := Response{}

	respIndex := 0

	// header
	parsedResponse.transactionID = int(binary.BigEndian.Uint16(response[respIndex : respIndex+2]))
	respIndex += 2

	flags := binary.BigEndian.Uint16(response[respIndex : respIndex+2])
	parsedResponse.authoritative = (flags>>10)&0x1 == 1
	parsedResponse.truncated = (flags>>9)&0x1 == 1 // TODO: handle truncation
	parsedResponse.responseCode = uint8(flags & 0x000f)
	respIndex += 2

	qCount := binary.BigEndian.Uint16(response[respIndex : respIndex+2])
	respIndex += 2

	ansCount := binary.BigEndian.Uint16(response[respIndex : respIndex+2])
	respIndex += 2

	_ = binary.BigEndian.Uint16(response[respIndex : respIndex+2]) // number of authority RRs
	respIndex += 2

	_ = binary.BigEndian.Uint16(response[respIndex : respIndex+2]) // number of additional RRS
	respIndex += 2

	// questions
	qDomains := []string{}
	for range qCount {
		domain, next := readDNSName(response, respIndex)
		respIndex = next
		_ = binary.BigEndian.Uint16(response[respIndex : respIndex+2]) // type
		respIndex += 2
		_ = binary.BigEndian.Uint16(response[respIndex : respIndex+2]) // class
		respIndex += 2
		qDomains = append(qDomains, domain)
	}

	parsedResponse.domains = qDomains

	// answers
	responseAnswers := []DNSAnswer{}
	for range ansCount {
		answer := DNSAnswer{}
		_, next := readDNSName(response, respIndex)
		respIndex = next

		answer.Type = binary.BigEndian.Uint16(response[respIndex : respIndex+2])
		respIndex += 2
		answer.Class = binary.BigEndian.Uint16(response[respIndex : respIndex+2])
		respIndex += 2

		answer.TTL = binary.BigEndian.Uint32(response[respIndex : respIndex+4])
		respIndex += 4

		dataLen := binary.BigEndian.Uint16(response[respIndex : respIndex+2])
		answer.DataLen = dataLen
		respIndex += 2

		if answer.Type == 1 && dataLen == 4 {
			var ipBuilder strings.Builder
			for i := range dataLen {
				if i > 0 {
					ipBuilder.WriteByte('.')
				}
				fmt.Fprintf(&ipBuilder, "%d", response[respIndex])
				respIndex++
			}
			answer.IP = ipBuilder.String()
			parsedResponse.hasAddress = true
		} else if answer.Type == 28 && dataLen == 16 {
			var ipBuilder strings.Builder
			for i := range dataLen / 2 {
				if i > 0 {
					ipBuilder.WriteByte(':')
				}
				fmt.Fprintf(&ipBuilder, "%02x%02x", response[respIndex], response[respIndex+1])
				respIndex += 2
			}
			answer.IP = ipBuilder.String()
			parsedResponse.hasAddress = true
		} else if answer.Type == 5 {
			cname, cnameNext := readDNSName(response, respIndex)
			respIndex = cnameNext
			answer.CNAME = cname
			if parsedResponse.firstCNAME == "" {
				parsedResponse.firstCNAME = cname
			}
		} else {
			respIndex += int(dataLen)
		}

		responseAnswers = append(responseAnswers, answer)
	}

	parsedResponse.answers = responseAnswers

	return parsedResponse
}

func qTypeName(t uint16) string {
	switch t {
	case 1:
		return "A"
	case 5:
		return "CNAME"
	case 28:
		return "AAAA"
	default:
		return "UNKNOWN"
	}
}

func qClassName(class uint16) string {
	switch class {
	case 1:
		return "IN"
	default:
		return "UNKNOWN"
	}
}

func rcodeName(code uint8) string {
	switch code {
	case 0:
		return "NOERROR"
	case 1:
		return "FORMERR"
	case 2:
		return "SERVFAIL"
	case 3:
		return "NXDOMAIN"
	case 4:
		return "NOTIMP"
	case 5:
		return "REFUSED"
	default:
		return "UNKNOWN"
	}
}

func printResponse(response Response) {
	fmt.Printf("transaction_id: %d\n", response.transactionID)
	fmt.Printf("domains: %v\n", response.domains)
	fmt.Printf("flags: authoritative=%t truncated=%t rcode=%d(%s)\n",
		response.authoritative,
		response.truncated,
		response.responseCode,
		rcodeName(response.responseCode),
	)
	fmt.Println("answers:")
	if len(response.answers) == 0 {
		fmt.Println("  (none)")
		return
	}

	for i, answer := range response.answers {
		data := answer.IP
		if answer.CNAME != "" {
			data = answer.CNAME
		}
		if data == "" {
			data = "-"
		}
		fmt.Printf("  %d) data=%s type=%d(%s) class=%d(%s) ttl=%dms data_len=%d\n",
			i+1,
			data,
			answer.Type,
			qTypeName(answer.Type),
			answer.Class,
			qClassName(answer.Class),
			answer.TTL,
			answer.DataLen,
		)
	}
	fmt.Println()
}

func qTypeFromString(recordType string) uint16 {
	switch strings.ToLower(recordType) {
	case "a":
		return 1
	case "cname":
		return 5
	case "aaaa":
		return 28
	default:
		fmt.Fprintf(os.Stderr, "Unsupported record type: %s\n", recordType)
		os.Exit(1)
		return 0
	}
}

func getRecordRecursive(domain string, dns string, recordType string, depth int) Response {
	if depth > 5 {
		fmt.Fprintf(os.Stderr, "CNAME recursion limit reached for %s\n", domain)
		os.Exit(1)
	}

	query := DNSQuery{Domain: domain, Type: qTypeFromString(recordType), Class: 1}
	transactionID, msg := constructRequest(query)

	conn, err := net.Dial("udp", dns+":53")
	if err != nil {
		panic(err)
	}

	n, err := conn.Write(msg)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error while writing %v\n", err.Error())
		return Response{}
	}
	if n != len(msg) {
		fmt.Fprintf(os.Stderr, "Wrote %d out of %d\n", n, len(msg))
		return Response{}
	}

	response := make([]byte, 512)
	_, err = conn.Read(response)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error while reading %v\n", err.Error())
		return Response{}
	}

	parsedResponse := parseResponse(response)
	if parsedResponse.domains[len(parsedResponse.domains)-1] != domain {
		fmt.Fprintf(os.Stderr, "Requested for %s, got %s\n", domain, parsedResponse.domains)
		return Response{}
	}

	if parsedResponse.transactionID != transactionID {
		fmt.Fprintf(os.Stderr, "Requested with transaction ID: %d, got %d\n", transactionID, parsedResponse.transactionID)
		return Response{}
	}

	if parsedResponse.hasAddress {
		return parsedResponse
	}

	if !parsedResponse.hasAddress && parsedResponse.firstCNAME != "" {
		fmt.Printf("No direct %s record for %s, found CNAME %s\n", strings.ToUpper(recordType), domain, parsedResponse.firstCNAME)
		return getRecordRecursive(parsedResponse.firstCNAME, dns, recordType, depth+1)
	}

	if query.Type == 1 {
		fmt.Fprintf(os.Stderr, "No A/CNAME record found for %s\n", domain)
	} else {
		fmt.Fprintf(os.Stderr, "No AAAA/CNAME record found for %s\n", domain)
	}
	return parsedResponse
}

func getRecord(domain string, dns string, recordType string) Response {
	return getRecordRecursive(domain, dns, recordType, 0)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stdout, "Usage: %v <domain_name> <optional_ipv4_dns_resolver>\n", os.Args[0])
		fmt.Fprintf(os.Stdout, "  %v google.com\n", os.Args[0])
		fmt.Fprintf(os.Stdout, "  %v google.com 8.8.4.4\n\n", os.Args[0])
		fmt.Fprintf(os.Stderr, "No arguments found\n")
		os.Exit(1)
	}

	domain := os.Args[1]

	dns := "1.1.1.1"
	if len(os.Args) > 2 {
		dns = os.Args[2]
	}

	// TODO: remove starting / and ending / (https://www.google.com -> www.google.com)
	// u, err := url.Parse(domain)
	// if err != nil {
	// 	fmt.Printf("Could not parse url %s: %s\n", domain, err.Error())
	// }
	// domain = u.Hostname()
	// fmt.Println(domain)

	if domain[len(domain)-1] != '.' {
		domain += "."
	}
	domain = strings.ToLower(domain)
	if len(domain) >= 256 {
		fmt.Printf("Domain name %s exceeds limit of 256 bytes\n", domain)
		os.Exit(1)
	}

	var parsedResponseA Response
	var parsedResponseAAAA Response
	var wg sync.WaitGroup
	wg.Go(func() {
		fmt.Printf("Asking %s for A records of %s\n", dns, domain)
		parsedResponseA = getRecord(domain, dns, "A")
	})
	wg.Go(func() {
		fmt.Printf("Asking %s for AAAA records of %s\n", dns, domain)
		parsedResponseAAAA = getRecord(domain, dns, "AAAA")
	})

	wg.Wait()

	fmt.Println()
	fmt.Println("Response A record:")
	printResponse(parsedResponseA)
	fmt.Println("Response AAAA record:")
	printResponse(parsedResponseAAAA)
}
