import generatePaths from "./generator.js";

let data5k = generatePaths(5000);
let data1k = generatePaths(1000);

const data = data1k;
const DATA_COUNT = data.length;

let data_div_p = document.querySelector("#data_div>p");
data_div_p.innerText = `List of all data (${DATA_COUNT})`;

let data_div_ol = document.createElement("ol");
data_div_ol.setAttribute("id", "data_div_ol");
data_div.appendChild(data_div_ol);

let data_list = document.createElement("datalist");
data_list.setAttribute("id", "data-list"); // matching list in input tag
search_div.appendChild(data_list);

let data_div_ul_html = '';
let data_list_html = '';

for (let str of data) {
    data_div_ul_html += `<li>${str}</li>`;
    data_list_html += `<option value=${str} />`;
}
data_div_ol.innerHTML = data_div_ul_html;
data_list.innerHTML = data_list_html;


function debounce(f, delay) {
    let timerId;

    return function(...args) {
        clearTimeout(timerId);

        timerId = setTimeout(function() {f(...args);}, delay);
    }
}

let search_method = "final";

let selected_text = '';
let current_li = null;
let results_count = 0;

let results_ul =  document.createElement("ul");
results_ul.setAttribute("id", "results_ul");
results_div.appendChild(results_ul);

function levenshtein(a, b) {
    if (a === b) return 0;

    let m = a.length;
    let n = b.length;

    if (n > m) {
        [a, b] = [b, a];
        [m, n] = [n, m];
    }

    let prev = new Uint32Array(n + 1);
    let curr = new Uint32Array(n + 1);

    for (let j = 0; j <= n; j++)
        prev[j] = j;

    for (let i = 1; i <= m; i++) {
        curr[0] = i;

        const ca = a.charCodeAt(i - 1);

        for (let j = 1; j <= n; j++) {
            const cost = ca === b.charCodeAt(j - 1) ? 0 : 1;

            const del = prev[j] + 1;
            const ins = curr[j - 1] + 1;
            const sub = prev[j - 1] + cost;

            curr[j] = del < ins ? (del < sub ? del : sub) : (ins < sub ? ins : sub);
        }

        [prev, curr] = [curr, prev];
    }

    return prev[n];
}

function damerauLevenshtein(a, b) {
    const m = a.length;
    const n = b.length;

    if (m === 0) return n;
    if (n === 0) return m;

    const INF = m + n;

    const H = Array.from({ length: m + 2 }, () => new Int32Array(n + 2));

    H[0][0] = INF;

    for (let i = 0; i <= m; i++) {
        H[i + 1][1] = i;
        H[i + 1][0] = INF;
    }

    for (let j = 0; j <= n; j++) {
        H[1][j + 1] = j;
        H[0][j + 1] = INF;
    }

    const lastRow = new Int32Array(128);

    for (let i = 1; i <= m; i++) {
        let lastMatchCol = 0;

        const ai = a.charCodeAt(i - 1);

        for (let j = 1; j <= n; j++) {
            const bj = b.charCodeAt(j - 1);

            const i1 = lastRow[bj];
            const j1 = lastMatchCol;

            let cost = 1;
            if (ai === bj) {
                cost = 0;
                lastMatchCol = j;
            }

            const transposition =
                H[i1][j1] +
                (i - i1 - 1) +
                1 +
                (j - j1 - 1);

            H[i + 1][j + 1] = Math.min(
                H[i][j] + cost,
                H[i + 1][j] + 1,
                H[i][j + 1] + 1,
                transposition
            );
        }

        lastRow[ai] = i;
    }

    return H[m + 1][n + 1];
}

function is_char_alnum(char) {
  const code = char.charCodeAt(0);
  
  return (
    (code >= 48 && code <= 57) ||
    (code >= 65 && code <= 90) ||
    (code >= 97 && code <= 122)
  );
}

function make_array_of_str(str) {
    let str_array = [];
    let temp_str = "";
    let temp_str_l = 0;
    let last_type = -1; // 0 means lowercase, 1 means upper case, 2 means digit
    for (let i = 0; i < str.length; i++) {
        let char  = str[i];
        let curr_type = -1;
        const code = char.charCodeAt(0);
        if (code >= 97 && code <= 122) {
            curr_type = 0;
        } else if (code >= 65 && code <= 90) {
            curr_type = 1;
            char = char.toLowerCase();
        } else if (code >= 48 && code <= 57) {
            curr_type = 2;
        }

        if (char === ' ' || char === '\n' || char === '\r' || char === '\t') {
            if (temp_str_l > 0) {
                str_array.push(temp_str);
                temp_str_l = 0;
                temp_str = "";
            }
        } else if (curr_type >= 0) {
            if (
                (curr_type === 2 && last_type !== 2) ||
                (curr_type === 1 && last_type === 0)
            ) {
                if (temp_str_l > 0) {
                    str_array.push(temp_str);
                    temp_str_l = 0;
                    temp_str = "";
                }
            }
            temp_str_l ++;
            temp_str += char;
        } else {
            if (temp_str_l > 0) {
                str_array.push(temp_str);
                temp_str_l = 0;
                temp_str = "";
            }
        }
        last_type = curr_type;
    }
    if (temp_str_l > 0) {
        str_array.push(temp_str);
    }
    return str_array;
}

// least to towards edges, max at around 80% then again reduce
// more emphasis to file name, then extension then folder/path
function position_score(i, len) {
    const t = i / (len - 1);

    let weight;
    if (t < 0.8)
        weight = 0.4 + 0.8 * t;
    else
        weight = 1.04 - 0.3 * (t - 0.8) / 0.2;

    return weight;
}

// arr1 is the search term,arr2 the string to search in
function match_score(arr1, arr2) {
    let match_score = 0;
    let last_match = -1;
    let matched = 0;
    let found2 = Array(arr2.length).fill(false);
    for (let fa of arr1) {
        let current_match_score = 0;
        let best_i = -1;
        let best_l = Infinity;
        for (let i = 0; i < arr2.length; i++) {
            let l = 10000000;
            if (arr2[i] === fa) {
                best_l = 0;
                best_i = i;
                break;
            }
            const index = arr2[i].indexOf(fa);
            if (index === 0) {
                l = (arr2[i].length - fa.length ) / (arr2[i].length);
            } else if (index > 0) {
                l = (arr2[i].length + index) / (2*arr2[i].length);
            } else {
                if (Math.abs(fa.length - arr2[i].length) > 3) {
                    l = 1;
                } else {
                    l =
                        damerauLevenshtein(fa, arr2[i])/Math.max(fa.length, arr2[i].length);
                }
            }
            if (l < best_l) {
                best_l = l;
                best_i = i;
            }
        }
        // console.log(best_l);
        if (best_l > 0.7) {
            current_match_score += 0;
            continue;
        }

        if (best_l === 0) {
            current_match_score += 100;
        } else {
            current_match_score += 100 * (1 - best_l * best_l);
        }

        if (last_match === -1) {
            last_match = best_i;
        } else {
            // add points for correct order, and close by
            if (best_i <= last_match)
                current_match_score -= 20;
                else {
                    current_match_score += Math.max(30 - (best_i - last_match - 1) * 2, 0);
                    if ((best_i - last_match) === 1) { // a bit more score for consecutive match
                        current_match_score += 20;
                    }
                    last_match = best_i;
                }

            if (!found2[best_i]) { // not repeat find
                current_match_score += 50;
                found2[best_i] = true;
            }
        }

        // matched another search token
        matched ++;

        // IMP: specifically for file paths, matching file names (ignoring other path, ext) is bonus
        current_match_score += 50*position_score(best_i, arr2.length);

        match_score += current_match_score * fa.length;
    }
    if (matched === arr1.length) {
        match_score += 30;
    }
    return match_score;
}

function search(field) {
    output.textContent = '';
    results_count = 0;
    current_li = null;

    if (search_method === "html")
        return; // data-list will handle this

    // search field in data and populat results_ul
    if (field === "") {
        results_ul.textContent = "";
        return;
    }
    output.textContent = "Searching...";

    let start = performance.now();

    let html = "";
    if (search_method === "substring") {
        for (let str of data) {
            if (str.includes(field)) {
                html += `<li class="result_li">${str}</li>`;
                results_count++;
            }
        }
    } else if (search_method === "regex") {
        const regex = new RegExp(field, "i");

        for (let str of data) {
            if (regex.test(str)) {
                html += `<li class="result_li">${str}</li>`;
                results_count++;
            }
        }
    } else if (search_method === "fuzzysort") {
        const results = fuzzysort.go(field, data, {threshold:0.3});
        for (let result of results) {
            html += `<li class="result_li">${result.target}</li>`;
            results_count++;
        }
    } else if (search_method === "levenshtein") {
        let levedist = [];
        field = field.toLowerCase();
        for (let i = 0; i < data.length; i++) {
            let str = data[i];
            if (str) {
                const d = levenshtein(str.toLowerCase(), field);
                if (d > str.length*0.90) continue;
                levedist.push({i: i, d: d});
            }
        }
        levedist.sort((a,b) => a.d - b.d);
        for (let obj of levedist) {
            html += `<li class="result_li">${data[obj.i]}</li>`;
        }
        results_count = levedist.length;
    } else if (search_method === "final") {
        let field_array = make_array_of_str(field);

        // console.log(field_array);
        let scores = [];
        for (let i = 0; i < data.length; i++) {
            const str = data[i];
            if (!str) continue;
            let str_array = make_array_of_str(str);
            let score = match_score(field_array, str_array);
            scores.push({i, score});
            // scores.push({str, score});
        }
        scores.sort((a,b)=>b.score-a.score);
        for (let obj of scores) {
            if (obj.score <= 0) break;
            html += `<li class="result_li">${data[obj.i]}</li>`;
            // html += `<li class="result_li">${obj.str}</li>`;
        }
        results_count = scores.length;
        // console.log(scores);
    }

    results_ul.innerHTML = html;
    let duration = performance.now() - start; // including html rendering time

    if (results_count > 0) {
        current_li = results_ul.firstElementChild;
        current_li.classList.add("result_li_hover");
        current_li.scrollIntoView({
          block: 'nearest',
          inline: 'nearest'
        });
    }

    output.textContent = `Took ${duration}ms to show ${results_count} items`;
}

let searchDebounce = debounce(search, 500);
function onChange(e) {
    searchDebounce(e.target.value);
}

function select(text) {
    selected_text = `Selected ${text}`;
    output.textContent = selected_text;
    results_ul.textContent = "";
    input.value = "";
    current_li = null;
}

input.addEventListener("input", onChange);

search_type.addEventListener("click", function (e) {
    if (e.target.type == "radio") {
        current_li = null;
        search_method = e.target.value;
        results_ul.textContent = "";
        output.textContent = "";
        if (search_method === "html") {
            input.setAttribute("list", "data-list");
        } else {
            input.setAttribute("list", "");
            searchDebounce(input.value);
        }
    }
});

results_ul.addEventListener("mousemove", function (e) {
    const li = e.target.closest(".result_li");
    if (!li) return;
    if (current_li !== li) {
        li.classList.add("result_li_hover");
        if (current_li) {
            current_li.classList.remove("result_li_hover");
        }
        current_li = li;
    }
});

results_ul.addEventListener("click", function (e) {
    e.preventDefault();
    // closest parent(or self)  closest because it will work if li had a <p> on which you hovered
    const li = e.target.closest(".result_li");
    if (!li) return;
    current_li = li;

    select(current_li.textContent);
});

search_container.addEventListener("keydown", (e) => {
    if (e.key === "ArrowDown") {
        e.preventDefault();

        if (!results_ul.firstElementChild) return; // no results yet
        if (current_li) {
            current_li.classList.remove("result_li_hover");
            // cycle
            current_li = current_li.nextElementSibling || results_ul.firstElementChild;
        } else {
            current_li = results_ul.firstElementChild;
        }
        current_li.classList.add("result_li_hover");
        current_li.scrollIntoView({ block: 'nearest', inline: 'nearest' });
    } else if (e.key === "ArrowUp") {
        e.preventDefault();

        if (!results_ul.firstElementChild) return; // no results yet
        if (current_li) {
            current_li.classList.remove("result_li_hover");
            // cycle
            current_li = current_li.previousElementSibling || results_ul.lastElementChild;
        } else {
            current_li = results_ul.lastElementChild;
        }
        current_li.classList.add("result_li_hover");
        current_li.scrollIntoView({ block: 'nearest', inline: 'nearest' });
    } else if (e.key === "Enter") {
        e.preventDefault();

        if (current_li) {
            select(current_li.textContent);
        }
    }
});
