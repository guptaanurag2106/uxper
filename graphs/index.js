const canvas = document.getElementById("canvas");
if (!canvas.getContext) {
    throw new Error("canvas does not support context");
}

canvas.width = 600;
canvas.height = 600;

const ctx = canvas.getContext("2d", { alpha: true });

let width = 600;
let height = 600;

// shared measureText cache
const text_width_map = new Map();

const offscreen = document.createElement("canvas");
offscreen.width = width;
offscreen.height = height;
const octx = offscreen.getContext("2d");

const canvas_container = document.getElementById("canvas-container")

const observer = new ResizeObserver(() => {
    const w = Math.floor(canvas_container.offsetWidth);
    if (w === 0 || w === width) return;
    canvas_container.style.height = `${w}px`;
    width = w;
    height = w;
    canvas.width = width;
    canvas.height = height;
    offscreen.width = width;
    offscreen.height = height;
    apply_font();
    render_static();
    blit();
});

observer.observe(canvas_container);

let reset_button = document.getElementById("reset");
let count_input = document.getElementById("count");
let download_button = document.getElementById("download");
download_button.onclick = e => {
    const imageURI = canvas.toDataURL("image/png");

    const link = document.createElement("a");
    link.download = "graph.png";
    link.href = imageURI;

    link.click();
};

function test_functionx(x) {
    if (x < 10) {
        return 10 * Math.sin(x * 0.8);
    }

    if (x < 20) {
        const t = (x - 10) / 10;
        const amplitude = 10 + t * 40;

        return amplitude * Math.sin(x * 0.65) + 2;
    }

    if (x < 30) {
        let y =
            45 * Math.sin(x * 0.55) +
            5 * Math.cos(x * 0.2) +
            Math.sin(x * 17.123) * 8;

        if (x === 22) y += 35;
        if (x === 25) y -= 45;
        if (x === 28) y += 30;

        return Math.max(-100, Math.min(100, y));
    }

    const t = x - 30;
    const baseline = 200 + t * 8;

    let y =
        baseline +
        45 * Math.sin(x * 0.45) +
        12 * Math.cos(x * 0.9);

    //random spikes
    if (x === 34) y += 60;
    if (x === 39) y -= 80;
    if (x === 45) y += 70;
    if (x === 52) y -= 60;
    if (x === 58) y += 50;

    return y;
}

function test_function(x) {
    return test_functionx(x) / 1000;
}

let data = []; // data will be [[0,func(0)],[1,func(1)]...]
const initial_len = 10;
for (let i = 0; i < initial_len; i++) {
    data.push([i, test_function(i)]);
}
count_input.value = initial_len;

function draw_borders() {
    ctx.lineWidth = 3;
    ctx.strokeStyle = "black";
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(0, height);
    ctx.lineTo(width, height);
    ctx.lineTo(width, 0);
    ctx.lineTo(0, 0);
    ctx.stroke();
}

function draw_hover_line([x, y]) {
    ctx.lineWidth = 1;
    ctx.strokeStyle = "grey";
    ctx.beginPath();
    ctx.setLineDash([5, 5])
    ctx.moveTo(0, y);
    ctx.lineTo(width, y);
    ctx.moveTo(x, 0);
    ctx.lineTo(x, height);
    ctx.stroke();
    ctx.setLineDash([]);
}

const CIRCLE_RADIUS = width / 150;
// monokai style LLM generated
const COL_BG = "#272822";
const COL_GRID = "#49483e";
const COL_TEXT = "#f8f8f2";
const COL_LINE = "#66d9ef";
const COL_POINT = "#a6e22e";
const COL_HOVER = "#e6db74";

let MAX_POINTS = width / 10;
let scale_x;
let scale_y;
// plot area geometry, updated every render_static, shared with input handlers
let pad_x, pad_y, plot_w, plot_h;
let data_x_range;
let data_y_range;
let screen_coords = [];

let font_size = width / 45;
// re-derive size-dependent values; cached text widths measured under the old size go stale
function apply_font() {
    font_size = width / 45;
    octx.font = `${font_size}px ${octx.font.split(" ").slice(1).join(" ")}`;
    ctx.font = `${font_size}px ${ctx.font.split(" ").slice(1).join(" ")}`;
    text_width_map.clear();
}
apply_font();

let current_view = [];

let dragging = false;
let last_x = 0;

function tick_label(val, step) {
    const d = Math.max(0, Math.ceil(-Math.log10(step)));
    let s = val.toFixed(d);
    // strip fractional zeros only, keep integer zeros 10 100
    if (d > 0) s = s.replace(/(\.\d*?)0+$/, "$1").replace(/\.$/, "");
    return s === "-0" ? "0" : s;
}

// render from range a to be only both inclusive, by default render all
function render_static(view) {
    if (view === undefined) {
        view = [0, data.length - 1];
        reset_button.style.display = "none";
    }

    if (view[0] < 0) view[0] = 0;
    if (view[0] >= data.length) view[0] = data.length - 1;
    if (view[1] < 0) view[1] = 0;
    if (view[1] >= data.length) view[1] = data.length - 1;

    if (view[1] - view[0] >= MAX_POINTS) {
        view[0] = view[1] - MAX_POINTS + 1;
    }
    current_view = view;

    octx.fillStyle = COL_BG;
    octx.fillRect(0, 0, width, height);
    // draw_borders();
    // data format, x is just increasing

    if (data.length === 0 || view[0] > view[1]) {
        data_x_range = [-5, 5];
        data_y_range = [-5, 5];
        view = [0, -1];
        current_view = view;
    } else {
        data_x_range = [data[view[0]][0], data[view[1]][0]];
        data_y_range = [Infinity, -Infinity];
        for (let i = view[0]; i <= view[1]; i++) {
            data_y_range[0] = Math.min(data_y_range[0], data[i][1]);
            data_y_range[1] = Math.max(data_y_range[1], data[i][1]);
        }

        if (data_x_range[0] === data_x_range[1]) { // just 1 point
            if (data_x_range[0] === 0) {
                data_x_range = [-5, 5];
            } else {
                data_x_range = [Math.min(0, 2 * data_x_range[0]), Math.max(0, 2 * data_x_range[0])];
            }

            if (data_y_range[0] === 0) {
                data_y_range = [-5, 5];
            } else {
                data_y_range = [Math.min(0, 2 * data_y_range[0]), Math.max(0, 2 * data_y_range[0])];
            }

        } else if (data_y_range[0] === data_y_range[1]) { // more than 1 point but all y are same
            if (data_y_range[0] === 0) {
                data_y_range = [-5, 5];
            } else {
                data_y_range = [Math.min(0, 2 * data_y_range[0]), Math.max(0, 2 * data_y_range[0])];
            }
        }
    }

    // data_x_range = [Math.round(data_x_range[0] * 10) / 10, Math.round(data_x_range[1] * 10) / 10]
    // data_y_range = [Math.round(data_y_range[0] * 10) / 10, Math.round(data_y_range[1] * 10) / 10]

    const BARS = 10;

    // tick steps, needed for padding measurement too
    const span_x = data_x_range[1] - data_x_range[0];
    const span_y = data_y_range[1] - data_y_range[0];
    const step_x = span_x / BARS;
    const step_y = span_y / BARS;

    // plot area padding, labels in the padding
    const char_y_min = tick_label(data_y_range[0], step_y);
    const char_y_max = tick_label(data_y_range[1], step_y);
    let char_y_min_w = text_width_map.get(char_y_min);
    let char_y_max_w = text_width_map.get(char_y_max);
    if (char_y_min_w === undefined) {
        char_y_min_w = octx.measureText(char_y_min).width;
        text_width_map.set(char_y_min, char_y_min_w);
    }
    if (char_y_max_w === undefined) {
        char_y_max_w = octx.measureText(char_y_max).width;
        text_width_map.set(char_y_max, char_y_max_w);
    }
    pad_x = Math.max(Math.max(char_y_min_w, char_y_max_w) + font_size * 0.7, font_size * 2);
    pad_y = font_size * 1.5;
    plot_w = width - pad_x * 2;
    plot_h = height - pad_y * 2;


    scale_x = plot_w / (data_x_range[1] - data_x_range[0]);
    scale_y = plot_h / (data_y_range[1] - data_y_range[0]);

    screen_coords = new Array(view[1] - view[0] + 1);
    for (let i = view[0]; i <= view[1]; i++) {
        screen_coords[i - view[0]] = {
            sx: pad_x + (data[i][0] - data_x_range[0]) * scale_x,
            sy: height - pad_y - (data[i][1] - data_y_range[0]) * scale_y,
            di: i,
        };
    }

    // axes
    // grid
    octx.strokeStyle = COL_GRID;
    octx.lineWidth = 1;
    octx.beginPath();
    for (let k = 0; k <= BARS; k++) {
        const pos_x = pad_x + (k * step_x) * scale_x;
        octx.moveTo(Math.round(pos_x) + 0.5, Math.round(pad_y) + 0.5);
        octx.lineTo(Math.round(pos_x) + 0.5, Math.round(height - pad_y) + 0.5);
    }
    for (let k = 0; k <= BARS; k++) {
        const pos_y = height - pad_y - (k * step_y) * scale_y;
        octx.moveTo(Math.round(pad_x) + 0.5, Math.round(pos_y) + 0.5);
        octx.lineTo(Math.round(width - pad_x) + 0.5, Math.round(pos_y) + 0.5);
    }
    octx.stroke();

    // line segments
    if (screen_coords.length > 1) {
        octx.strokeStyle = COL_LINE;
        octx.lineWidth = 2;
        octx.beginPath();
        octx.moveTo(Math.round(screen_coords[0].sx) + 0.5, Math.round(screen_coords[0].sy) + 0.5);
        for (let i = 1; i < screen_coords.length; i++) {
            octx.lineTo(Math.round(screen_coords[i].sx) + 0.5, Math.round(screen_coords[i].sy) + 0.5);
        }
        octx.stroke();
    }

    // points
    octx.fillStyle = COL_POINT;
    octx.beginPath();
    for (let i = 0; i < screen_coords.length; i++) {
        octx.moveTo(screen_coords[i].sx + CIRCLE_RADIUS, screen_coords[i].sy);
        octx.arc(screen_coords[i].sx, screen_coords[i].sy, CIRCLE_RADIUS, 0, Math.PI * 2);
    }
    octx.fill();

    // labels (in the padding)
    const gap = font_size * 0.35;
    octx.fillStyle = COL_TEXT;
    octx.textAlign = "center";
    octx.textBaseline = "top";
    for (let k = 0; k <= BARS; k++) {
        const x = data_x_range[0] + k * step_x;
        const pos_x = pad_x + (x - data_x_range[0]) * scale_x;
        octx.fillText(tick_label(x, step_x), pos_x, height - pad_y + gap);
    }
    octx.textAlign = "right";
    octx.textBaseline = "middle";
    for (let k = 0; k <= BARS; k++) {
        const y = data_y_range[0] + k * step_y;
        const pos_y = height - pad_y - (y - data_y_range[0]) * scale_y;
        octx.fillText(tick_label(y, step_y), pad_x - gap, pos_y);
    }

    if (view[1] !== data.length - 1) {
        octx.fillStyle = "red";
        octx.textAlign = "left";
        octx.textBaseline = "top";
        octx.fillText('→', width - pad_x / 2, height - pad_y + gap);

    }
    if (view[0] !== 0) {
        octx.fillStyle = "red";
        octx.textAlign = "right";
        octx.textBaseline = "top";
        octx.fillText('←', pad_x / 2, height - pad_y + gap);
    }
}

function hover_point(point) {
    // since x is always increasing binary search
    const mx = point[0];
    let l = 0, r = screen_coords.length - 1;
    while (l < r) {
        const mid = l + ((r - l) >> 1);
        if (screen_coords[mid].sx < mx) l = mid + 1;
        else r = mid;
    }

    let best = -1, best_dist = Infinity;
    for (let i of [l - 1, l]) {
        if (i < 0 || i >= screen_coords.length) continue;
        const dist = Math.hypot(mx - screen_coords[i].sx, point[1] - screen_coords[i].sy);
        if (dist < best_dist) {
            best_dist = dist;
            best = i;
        }
    }

    if (best !== -1 && best_dist <= CIRCLE_RADIUS * 2) {// slightly increase radius for easy hover
        const { sx, sy, di } = screen_coords[best];
        ctx.fillStyle = COL_HOVER;
        ctx.textAlign = "left";
        ctx.textBaseline = "bottom";
        // flip label side if it would clip the padding
        const label = `${data[di][0]}, ${Number(data[di][1].toFixed(2))}`;
        let tx = sx + font_size * 0.4;
        if (tx + ctx.measureText(label).width > width - font_size) tx = sx - ctx.measureText(label).width - font_size * 0.4;
        let ty = sy - font_size * 0.4;
        if (ty - font_size < 0) ty = sy + font_size;
        ctx.fillText(label, tx, ty);
    }
}

console.log(data);
render_static();
ctx.drawImage(offscreen, 0, 0);

function blit() {
    ctx.clearRect(0, 0, width, height);
    ctx.drawImage(offscreen, 0, 0);
}

canvas.onpointerdown = e => {
    if (e.button != 0) return;
    dragging = true;
    last_x = e.clientX;
};

canvas.onpointermove = e => {
    canvas.style.cursor = "pointer";
    const r = canvas.getBoundingClientRect();
    const point = [e.clientX - r.left, e.clientY - r.top];
    blit();
    hover_point(point);

    if (!dragging) return;
    const dx = e.clientX - last_x;
    if (dx < 0) {
        if (current_view[1] != (data.length - 1)) {
            current_view[0]++;
            current_view[1]++;
            render_static(current_view);
            blit();
            reset_button.style.display = "block";
        }
    } else if (dx > 0) {
        if (current_view[0] != 0) {
            current_view[0]--;
            current_view[1]--;
            render_static(current_view);
            blit();
            reset_button.style.display = "block";
        }
    }
    last_x = e.clientX;
}

canvas.onpointerup = e => {
    if (e.button !== 0) return;

    dragging = false;
    canvas.releasePointerCapture(e.pointerId);
}

canvas.onwheel = e => {
    if (data.length === 0) return;
    const r = canvas.getBoundingClientRect();
    const mx = e.clientX - r.left;

    // lerp to find index in data where cursor is
    const t = Math.min(Math.max((mx - pad_x) / plot_w, 0), 1);
    const a = current_view[0], b = current_view[1];
    const span = b - a;
    const anchor = a + t * span;

    // zoomin dec span, zoomout inc span
    let s = e.deltaY < 0 ? span * 0.8 : span * 1.25;
    if (e.deltaY > 0 && s >= data.length - 1) {
        render_static(); // fully zoomed out
        blit();
        return;
    }
    s = Math.min(Math.max(s, 2), data.length - 1);

    // calc new start of current_view
    let na = Math.round(anchor - t * s);
    render_static([na, na + Math.round(s)]);
    blit();
    reset_button.style.display = "block";
}

canvas.onmouseleave = () => {
    canvas.style.cursor = "default";
    dragging = false;
}

canvas.ondblclick = _ => {
    render_static();
    blit();
}

reset_button.onclick = _ => {
    render_static();
    blit();
}


count_input.onchange = e => {
    data = [];
    for (let i = 0; i < e.target.value; i++) {
        data.push([i, test_function(i)]);
    }
    render_static();
    blit();
};
