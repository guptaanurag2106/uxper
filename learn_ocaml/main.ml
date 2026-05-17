let isSafe grid row col num= 
    if  Array.exists (fun x -> x = num) grid.(row) then false else
        if Array.exists (fun x -> x.(col) = num) grid then false else
            let start_row = (row - (row mod 3)) in
            let start_col = (col - (col mod 3)) in
            if  Array.exists
                    (fun x -> Array.exists (fun y -> y = num) (Array.sub x start_col 3))
                    (Array.sub grid start_row 3) then false
            else true


let solve_sudoku (grid: int array array) =
    let rec solve_sudoku' grid row col= 
        let n = Array.length grid in
        match row, col with
        | r, c when r = (n-1) && c=n -> true
        |_, c when c = n -> solve_sudoku' grid (row+1) 0
        | _ when grid.(row).(col) != 0 -> solve_sudoku' grid row (col+1)
        | _ -> if Array.exists
                    (fun x -> if isSafe grid row col x then (
                        grid.(row).(col)<-x ; solve_sudoku' grid row (col+1)) else false)
                    (Array.init n (fun i -> i+1))
               then true
               else (grid.(row).(col)<-0; false)

    in

    solve_sudoku' grid 0 0

let rec print_array (xs: int array): string = 
    String.concat ", " (Array.to_list (Array.map string_of_int xs))


let rec print_grid grid: unit = 
    print_endline (Array.fold_left (fun a x -> a ^ "\n" ^ (print_array x)) "" grid)

let () =
    let grid = [|
        [|1; 0; 0; 0; 3; 4; 0; 0; 8|]; 
      	[|0; 7; 0; 6; 8; 0; 0; 3; 0|]; 
      	[|0; 0; 8; 2; 1; 0; 7; 0; 4|];
        [|0; 5; 4; 0; 9; 0; 6; 8; 0|]; 
      	[|9; 1; 0; 5; 0; 8; 0; 2; 0|]; 
      	[|0; 8; 0; 3; 0; 0; 0; 0; 5|];
        [|3; 0; 5; 9; 0; 6; 8; 7; 1|]; 
      	[|0; 0; 6; 0; 0; 0; 0; 4; 0|]; 
        [|0; 0; 1; 0; 7; 0; 2; 0; 0|];
    |]  in
    if solve_sudoku grid then print_grid grid else print_endline "Sudoku not solvable"
