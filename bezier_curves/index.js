if (!canvas.getContext) {
    throw new Error("Canvas does not support context");
}
const ctx = canvas.getContext("2d", {alpha: true});
const width = canvas.width;
const height = canvas.height;
const canvasBound = canvas.getBoundingClientRect();
const topLeft = [canvasBound.top, canvasBound.left];

ctx.fillStyle = "red";
ctx.strokeStyle = "grey";

let points = []
const point_radius = 10
let points_changed = false
let redraw_pending = false

function request_redraw() {
    if (!redraw_pending) {
        redraw_pending = true;
        draw();
    }
}

function curve(start, end, t) { // start, end indices
    if (start > end) return []

    if (start == end) return points[start]

    let point1 = curve(start, end-1, t);
    let point2 = curve(start+1, end, t);

    let sx = point1[0], sy = point1[1];
    let ex = point2[0], ey = point2[1];
    return [(1-t)*sx + t*ex, (1-t)*sy + t*ey];
}

function lerp(point1, point2, t) {

    let sx = point1[0], sy = point1[1];
    let ex = point2[0], ey = point2[1];
    return [(1-t)*sx + t*ex, (1-t)*sy + t*ey];
}

function curve_casteljau(t) {
    let temp = structuredClone(points);

    let n = temp.length;
    while (n > 1) {
        for (let i = 0; i < n-1; i++) {
            temp[i] = lerp(temp[i], temp[i+1], t);
        }
        n--;
    }

    return temp[0];
}

function draw() {
    redraw_pending = false;

    ctx.clearRect(0, 0, width, height);
    ctx.beginPath()

    if (points.length > 0) {
        ctx.moveTo(points[0][0], points[0][1])

        ctx.setLineDash([10,5])
        for (let i = 1; i < points.length; i++) {
            ctx.lineTo(points[i][0], points[i][1])
        }
        ctx.stroke()

        ctx.beginPath()
        ctx.setLineDash([])
        ctx.moveTo(points[0][0], points[0][1])
        for (let t = 0; t <1; t+=0.01) {
            let curve_point = curve_casteljau(t)
            if (curve_point.length == 2)
                ctx.lineTo(curve_point[0], curve_point[1])
        }
        ctx.stroke()

        ctx.beginPath()

        ctx.fillStyle = "blue";
        let point = points[0];
        ctx.moveTo(point[0] + point_radius, point[1])
        ctx.arc(point[0], point[1], point_radius, 0, 2*Math.PI);
        ctx.fill();

        ctx.beginPath()
        ctx.fillStyle = "red";
        for (let index = 1; index < points.length-1; index++) {
            point = points[index];
            ctx.moveTo(point[0] + point_radius, point[1])
            ctx.arc(point[0], point[1], point_radius, 0, 2*Math.PI);
        }
        ctx.fill();

        ctx.beginPath()
        ctx.fillStyle = "blue";
        point = points[points.length-1];
        ctx.moveTo(point[0] + point_radius, point[1])
        ctx.arc(point[0], point[1], point_radius, 0, 2*Math.PI);

        ctx.fill();
    }

    // window.requestAnimationFrame(draw);
}

// window.requestAnimationFrame(draw);

let is_point_dragging = false;
let drag_index = -1;

canvas.addEventListener("mousedown", (e) => {
    for (let index = 0; index < points.length; index++) {
        if ((Math.pow((e.offsetX - points[index][0]), 2) + Math.pow((e.offsetY - points[index][1]), 2)) <= point_radius * point_radius) {
            is_point_dragging  = true;
            drag_index = index;
            return;
        }
    }
})

canvas.addEventListener("mouseup", (e) => {
    if (is_point_dragging && drag_index >=0 && drag_index < points.length) {
        is_point_dragging = false;
        drag_index = -1;
        request_redraw();
    } else {
        if (e.ctrlKey) {
            for (let index = 0; index < points.length; index++) {
                if ((Math.pow((e.offsetX - points[index][0]), 2) + Math.pow((e.offsetY - points[index][1]), 2)) <= point_radius * point_radius) {
                    points.splice(index, 1)
                    points_changed = true
                    request_redraw();
                }
            }
        } else {
            points.push([e.offsetX, e.offsetY])
            points_changed = true
            request_redraw();
        }
    }
})

canvas.addEventListener("mousemove", (e) => {
    if (is_point_dragging && drag_index >=0 && drag_index < points.length) {
        points[drag_index] = [e.offsetX, e.offsetY];
        request_redraw();
    }
})

canvas.addEventListener("click", (e) => {
    if (e.ctrlKey) { // ctrl+click may not register as mouseup
        for (let index = 0; index < points.length; index++) {
            if ((Math.pow((e.offsetX - points[index][0]), 2) + Math.pow((e.offsetY - points[index][1]), 2)) <= point_radius * point_radius) {
                points.splice(index, 1)
                points_changed = true
                request_redraw();
            }
        }
    }
})
