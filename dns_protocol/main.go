package main

import (
	"encoding/binary"
	"fmt"
	"math/rand"
	"net"
	"os"
	"strings"
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
	answers       []DNSAnswer
}

func parseResponse(response []byte) Response {
	parsedResponse := Response{}

	respIndex := 0

	// header
	parsedResponse.transactionID = int(binary.BigEndian.Uint16(response[respIndex : respIndex+2]))
	respIndex += 2

	flags := binary.BigEndian.Uint16(response[respIndex : respIndex+2])
	parsedResponse.authoritative = (flags>>10)&0x1 == 1
	parsedResponse.truncated = (flags>>9)&0x1 == 1
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
		var domainBuilder strings.Builder
		for {
			labelLen := response[respIndex]
			respIndex++
			if labelLen == 0 {
				break
			}
			domainBuilder.Write(response[respIndex : respIndex+int(labelLen)])
			domainBuilder.WriteByte('.')
			respIndex += int(labelLen)
		}
		_ = binary.BigEndian.Uint16(response[respIndex : respIndex+2])
		respIndex += 2
		_ = binary.BigEndian.Uint16(response[respIndex : respIndex+2])
		respIndex += 2
		qDomains = append(qDomains, domainBuilder.String())
	}

	parsedResponse.domains = qDomains

	// answers
	responseAnswers := []DNSAnswer{}
	for range ansCount {
		answer := DNSAnswer{}
		_ = binary.BigEndian.Uint16(response[respIndex : respIndex+2]) // name? 0xc00c
		respIndex += 2

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
	fmt.Println("Response:")
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
		ip := answer.IP
		if ip == "" {
			ip = "-"
		}
		fmt.Printf("  %d) ip=%s type=%d(%s) class=%d(%s) ttl=%dms data_len=%d\n",
			i+1,
			ip,
			answer.Type,
			qTypeName(answer.Type),
			answer.Class,
			qClassName(answer.Class),
			answer.TTL,
			answer.DataLen,
		)
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stdout, "Usage: %v <domain_name> <optional_ipv4_dns_resolver>\n", os.Args[0])
		fmt.Fprintf(os.Stdout, "  %v google.com\n", os.Args[0])
		fmt.Fprintf(os.Stdout, "  %v google.com 8.8.4.4\n\n", os.Args[0])
		fmt.Fprintf(os.Stderr, "No arguments found\n")
		os.Exit(1)
	}

	// TODO: remove starting / and ending / (https://www.google.com -> www.google.com)
	domain := os.Args[1]
	dns := "1.1.1.1"
	if len(os.Args) > 2 {
		dns = os.Args[2]
	}

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

	fmt.Printf("Asking %s for A records of %s\n", dns, domain)

	query := DNSQuery{
		Domain: domain,
		Type:   1,
		Class:  1,
	}

	transactionID, msg := constructRequest(query)

	conn, err := net.Dial("udp", dns+":53")
	if err != nil {
		panic(err)
	}

	n, err := conn.Write(msg)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error while writing %v\n", err.Error())
		os.Exit(1)
	}

	if n != len(msg) {
		fmt.Fprintf(os.Stderr, "Wrote %d out of %d\n", n, len(msg))
		os.Exit(1)
	}

	response := make([]byte, 512)
	_, err = conn.Read(response)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error while reading %v\n", err.Error())
		os.Exit(1)
	}

	// Msg Parse
	parsedResponse := parseResponse(response)
	if parsedResponse.domains[len(parsedResponse.domains)-1] != domain {
		fmt.Fprintf(os.Stderr, "Requested for %s, got %s\n", domain, parsedResponse.domains)
		os.Exit(1)
	}

	if parsedResponse.transactionID != transactionID {
		fmt.Fprintf(os.Stderr, "Requested with transaction ID: %d, got %d\n", transactionID, parsedResponse.transactionID)
		os.Exit(1)
	}

	printResponse(parsedResponse)
}
