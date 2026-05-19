let alpha_table =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

let alpha_index =
  Array.init 256 (fun code ->
      match String.index_opt alpha_table (Char.chr code) with
      | Some i -> i
      | None -> 0xFF)

let find_index c = alpha_index.(Char.code c)

let decode str =
  if String.length str mod 4 <> 0 then
    failwith "decode: not valid base64 encoded string"
  else
    let decode' c1 c2 c3 c4 =
      let b0 = find_index c1
      and b1 = find_index c2
      and b2 = find_index c3
      and b3 = find_index c4 in
      let final = (b0 lsl 18) lor (b1 lsl 12) lor (b2 lsl 6) lor b3 in
      ([ Char.chr ((final lsr 16) land 0xFF) ]
      @ (if b2 = 0xFF then [] else [ Char.chr ((final lsr 8) land 0xFF) ])
      @ if b3 = 0xFF then [] else [ Char.chr (final land 0xFF) ])
      |> List.to_seq |> String.of_seq
    in
    let rec decode_help ind =
      if ind >= String.length str then ""
      else
        decode' str.[ind] str.[ind + 1] str.[ind + 2] str.[ind + 3]
        ^ decode_help (ind + 4)
    in
    decode_help 0

let encode str =
  let len = String.length str in
  let rec encode' i =
    if i >= len then ""
    else
      let remaining = len - i in
      let b0 = Char.code str.[i] in
      let b1 = if remaining > 1 then Char.code str.[i + 1] else 0 in
      let b2 = if remaining > 2 then Char.code str.[i + 2] else 0 in
      let final = (b0 lsl 16) lor (b1 lsl 8) lor b2 in
      let c0 = alpha_table.[(final lsr 18) land 0x3F] in
      let c1 = alpha_table.[(final lsr 12) land 0x3F] in
      let c2 =
        if remaining > 1 then alpha_table.[(final lsr 6) land 0x3F] else '='
      in
      let c3 = if remaining > 2 then alpha_table.[final land 0x3F] else '=' in
      String.init 4 (function 0 -> c0 | 1 -> c1 | 2 -> c2 | _ -> c3)
      ^ encode' (i + 3)
  in
  encode' 0

let () =
  let args = Sys.argv in
  match Array.length args with
  | 2 -> print_endline (encode args.(1))
  | 3 -> (
      match args.(1) with
      | "-e" -> print_endline (encode args.(2))
      | "-d" -> print_endline (decode args.(2))
      | _ ->
          Printf.fprintf stderr
            "Usage: %s [-d|-e] <text>\nError got %s but should be -d or -e\n"
            args.(0) args.(1);
          exit 1)
  | _ ->
      Printf.fprintf stderr "Usage: %s [-d|-e] <text>\n" args.(0);
      exit 1
