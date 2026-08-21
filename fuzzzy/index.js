import generatePaths from "./generator.js";

let data5k = generatePaths(5000);
let data1k = generatePaths(10);

let data = [
    "/home/alice/projects/store-api/src/services/UserService.ts",
    "/home/alice/projects/store-api/src/services/UserService.test.ts",
    "/home/alice/projects/store-api/src/services/UserSessionService.ts",
    "/home/alice/projects/store-api/src/services/UserSearchService.ts",
    "/home/alice/projects/store-api/src/services/OrderService.ts",

    "/home/alice/projects/store-api/src/controllers/UserController.ts",
    "/home/alice/projects/store-api/src/controllers/UserSearchController.ts",
    "/home/alice/projects/store-api/src/controllers/OrderController.ts",

    "/home/alice/projects/store-api/src/repositories/UserRepository.ts",
    "/home/alice/projects/store-api/src/repositories/UserSearchRepository.ts",
    "/home/alice/projects/store-api/src/repositories/OrderRepository.ts",

    "/home/alice/projects/store-api/src/services/auth/UserAuthService.ts",
    "/home/alice/projects/store-api/src/services/auth/UserAuthorizationService.ts",
    "/home/alice/projects/store-api/src/services/users/UserProfileService.ts",

    "/home/alice/projects/store-api/tests/services/UserService.test.ts",
    "/home/alice/projects/store-api/tests/services/UserSearchService.test.ts",

    "/home/alice/projects/store-api/src/utils/SearchUtils.ts",
    "/home/alice/projects/store-api/src/utils/SearchUserUtils.ts",

    "/home/alice/projects/store-api/docs/UserSearchService.md",
    "/home/alice/projects/store-api/config/user-search.yml",
    "/home/alice/projects/store-api/src/search/user/services/OtherService.ts"
];
// data = data5k;
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

    return function (...args) {
        clearTimeout(timerId);

        timerId = setTimeout(function () { f(...args); }, delay);
    }
}

let search_method = "final";

let selected_text = '';
let current_li = null;
let results_count = 0;

let results_ul = document.createElement("ul");
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

function make_array_of_str(str) {
    let str_array = [];
    let temp_str = "";
    let last_type = -1; // 0 means lowercase, 1 means upper case, 2 means digit
    const str_len = str.length;
    for (let i = 0; i < str_len; i++) {
        let char = str[i];

        if (char === ' ' || char === '\n' || char === '\r' || char === '\t') {
            if (temp_str.length > 0) {
                str_array.push(temp_str);
                temp_str = "";
            }
        } else {
            let curr_type = -1;
            const code = char.charCodeAt(0);
            if (code >= 97 && code <= 122) {
                curr_type = 0;
            } else if (code >= 65 && code <= 90) {
                curr_type = 1;
                char = char.toLowerCase();
            } else if (code >= 48 && code <= 57) {
                curr_type = 2;
            } else {
                if (temp_str.length > 0) {
                    str_array.push(temp_str);
                    temp_str = "";
                }
                continue;
            }
            if (
                (curr_type === 1 && last_type === 0) ||
                (curr_type === 2 && last_type !== 2)
            ) {
                if (temp_str.length > 0) {
                    str_array.push(temp_str);
                    temp_str = "";
                }
            }
            temp_str += char;
            last_type = curr_type;
        }
    }
    if (temp_str.length > 0) {
        str_array.push(temp_str);
    }
    return str_array;
}

// least to towards edges, max at around 80% then again reduce
// more emphasis to file name, then extension then folder/path
function position_score(i, len) {
    const t = i / (len - 1);

    //so gradual increase from 0 to rise_end, then plateau around rise_end to fall_start, then fall down
    const rise_end = 0.70;
    const fall_start = 0.90;

    if (t < rise_end) {
        const x = t / rise_end;
        const s = x * x * (3 - 2 * x); // smoothstep
        return 0.5 + 0.5 * s;
    }

    if (t < fall_start) {
        return 1.0;
    }

    const x = (t - fall_start) / (1.0 - fall_start);
    const s = x * x * (3 - 2 * x);
    return 1.0 - 0.25 * s;
}

function match_score_parent(arr1) {
    const damerau_cache = new Map();

    function get_damerau(a, b) {
        const key = a + "\0" + b;
        let d = damerau_cache.get(key);
        if (d === undefined) {
            d = damerauLevenshtein(a, b);
            damerau_cache.set(key, d);
        }
        return d;
    }

    // arr1 is the search term,arr2 the string to search in
    function match_score(arr2) {
        let match_score = 0;
        let matched = 0;
        const arr2_len = arr2.length;
        let found2 = new Uint8Array(arr2_len);
        const matches = [];
        // console.log('------------', arr2.join('/'))
        let arr1_len = arr1.length;
        for (let i = 0; i < arr1_len; i++) {
            let fa = arr1[i];
            let fa_len = fa.length;
            let current_match_score = 0;
            let best_i = -1;
            let best_l = Infinity;
            let best_type = -1; // 0 exact, 1 prefix, 2 substring, 3 fuzzy

            let split_before = "";
            let split_after = "";
            for (let i = 0; i < arr2_len; i++) {
                const arr2_i = arr2[i];
                const arr2_i_len = arr2_i.length;
                let l = 10000000;
                if (arr2_i === fa) {
                    best_l = 0;
                    best_i = i;
                    best_type = 0;
                    break;
                }
                if (fa_len < arr2_i_len) {
                    const index = arr2_i.indexOf(fa);

                    if (index >= 0) {
                        const x = 1 - fa_len / arr2_i_len;
                        l = 0.6 * x * x + index / arr2_i_len;

                        if (l < best_l) {
                            best_l = l;
                            best_i = i;
                            best_type = 1;
                            continue;
                        }
                    }
                }

                const len_diff = fa_len - arr2_i_len;
                if (len_diff > 4 || len_diff < -4) {
                    l = 1;
                } else {
                    l =
                        get_damerau(fa, arr2_i) / Math.max(arr2_i_len, fa_len);
                    // console.log(fa, arr2_i, l);
                }
                if (l < best_l) {
                    best_l = l;
                    best_i = i;
                    best_type = 2;
                }
            }

            // const cap = fa.length <= 5 ? 0.35 : 0.6;
            const cap = 0.5;
            // if (best_type === 2 && best_l > cap) {
            if (best_l >= cap) {
                current_match_score += 0;
                match_score += current_match_score;
            } else {
                current_match_score += 80 * (1 - best_l * best_l);

                // Remember the match. Order/proximity is calculated after all
                // search tokens have been matched.
                matches.push(best_i);

                if (found2[best_i]) { // repeat find
                    current_match_score -= 30;
                } else {
                    found2[best_i] = true;
                }

                // IMP: specifically for file paths, matching file names (ignoring other path, ext) is bonus
                current_match_score += 15 * position_score(best_i, arr2_len);

                match_score += current_match_score * fa_len;
                // matched another search token
                matched++;
            }

            if (split_before.length > 0) {
                arr1_len++;
                arr1.push(split_before);
            }
            if (split_after.length > 0) {
                arr1_len++;
                arr1.push(split_after);
            }
        }
        const matches_len = matches.length;
        for (let i = 1; i < matches_len; i++) {
            const previous = matches[i - 1];
            const current = matches[i];

            // correct order
            if (current > previous) {
                match_score += 50;
            }

            // add points for being close
            const distance = Math.abs(current - previous) - 1;

            // proximity bonus, max 50
            match_score += Math.max(50 - distance * 3, 0);

            if (distance === 0) { // consecutive match
                match_score += 30;
            }
        }
        // penalize every query token that didn't match any candidate token:
        // an all-token match must outrank a partial one
        const matched_ratio = matched / arr1_len;
        match_score *= matched_ratio * matched_ratio;
        return match_score;
    }

    let scores = [];
    const data_len = data.length;
    for (let i = 0; i < data_len; i++) {
        const str = data[i];
        if (!str) continue;
        const str_array = make_array_of_str(str);
        const score = match_score(str_array);
        if (score != 0) {
            scores.push({ i, score });
            // scores.push({ str, score });
        }
    }
    return scores;
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
        const results = fuzzysort.go(field, data, { threshold: 0.3 });
        for (let result of results) {
            html += `<li class="result_li">${result.target}</li>`;
            results_count++;
        }
    } else if (search_method === "levenshtein") {
        let levedist = [];
        for (let i = 0; i < data.length; i++) {
            let str = data[i];
            if (str) {
                const d = levenshtein(str.toLowerCase(), field);
                if (d > str.length * 0.90) continue;
                levedist.push({ i: i, d: d });
            }
        }
        levedist.sort((a, b) => a.d - b.d);
        for (let obj of levedist) {
            html += `<li class="result_li">${data[obj.i]}</li>`;
        }
        results_count = levedist.length;
    } else if (search_method === "final") {
        let field_array = make_array_of_str(field);

        let scores = match_score_parent(field_array);
        scores.sort((a, b) => b.score - a.score);
        const max_score = scores[0].score;
        for (let obj of scores) {
            if (obj.score / max_score <= 0.5) break;
            html += `<li class="result_li">${data[obj.i]}</li>`;
            // html += `<li class="result_li">${obj.str}</li>`;
            results_count++;
        }
        console.log(scores)
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

let searchDebounce = debounce(search, 0);
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

// input.addEventListener("input", onChange);
input.addEventListener("keydown", (e) => {
    if (e.key === "Enter") {
        onChange(e);
    }
});

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
