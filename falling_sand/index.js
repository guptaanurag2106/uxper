if (!canvas.getContext) {
    throw new Error("Canvas does not support context");
}
const ctx = canvas.getContext("2d", {alpha: true});
const width = canvas.width;
const height = canvas.height;
const canvasBound = canvas.getBoundingClientRect();
const topLeft = [canvasBound.top, canvasBound.left];
ctx.strokeStyle = "grey";

const grid_size = 5;
const rows = height/grid_size;
const cols = width/grid_size;

class Empty {
}

class Sand {
    constructor(rand) {
        this.rand = rand;
    }

    colour() {
        return `hsl(${42}, ${35+this.rand}%, ${65+this.rand}%)`;
    }

    update() {
    }
}

class Wood {
    constructor(rand) {
        this.rand = rand;
    }

    colour() {
        return `hsl(${40}, ${40+this.rand}%, ${19+this.rand}%)`;
    }

    update() {
    }
}

class Smoke {
    constructor(rand, lifetime) {
        this.rand = rand;
        this.lightness = rand;
        this.lifetime = lifetime;
    }

    colour() {
        return `hsl(${78}, ${2+this.rand}%, ${35+this.lightness}%)`;
    }

    reduceLife() {
        this.lifetime --;
        this.lightness += 0.2;
        if (this.lifetime < 0) {
            return false;
        }
        return true;
    }

    update() {
    }
}

class Fire {
    constructor(rand, lifetime) {
        this.rand = rand;
        this.lifetime = lifetime
    }

    colour() {
        return `hsl(${20}, ${90+this.rand}%, ${45+this.rand}%)`;
    }

    reduceLife() {
        this.lifetime --;
        if (this.lifetime < 0) {
            return false;
        }
        return true;
    }

    update() {
    }
}

const EMPTY = new Empty();

let grid = Array(rows).fill().map(() => Array(cols).fill(EMPTY));

let mode = 1; // 1=>sand 2=>wood 3=>smoke

function draw() {
    ctx.clearRect(0,0,width,height);
    let new_grid = Array(rows).fill().map(() => Array(cols).fill(EMPTY));

    for (let i = 0; i < rows; i++) {
        for (let j = 0; j < cols; j++) {

            let val = grid[i][j];

            if (val instanceof Empty) continue;

            if (val instanceof Sand) {
                ctx.fillStyle = val.colour();
                ctx.fillRect(j*grid_size, i*grid_size, grid_size, grid_size);
                new_grid[i][j] = val;

                if (i<rows-1) {
                    if(grid[i+1][j] instanceof Empty) {
                        new_grid[i][j] = EMPTY;
                        new_grid[i+1][j] = val;
                    } else {
                        const dir = Math.sign(Math.random() - 0.5);
                        if (j+dir >= 0 && j+dir < cols && grid[i+1][j+dir] instanceof Empty) {
                            new_grid[i][j] = EMPTY;
                            new_grid[i+1][j+dir] = val;
                        } else if (j-dir >= 0 && j-dir < cols && grid[i+1][j-dir] instanceof Empty) {
                            new_grid[i][j] = EMPTY;
                            new_grid[i+1][j-dir] = val;
                        }
                    }
                }
            } else if (val instanceof Wood) {
                ctx.fillStyle = val.colour();
                ctx.fillRect(j*grid_size, i*grid_size, grid_size, grid_size);
                new_grid[i][j] = val;
            } else if (val instanceof Smoke) {
                ctx.fillStyle = val.colour();
                ctx.fillRect(j*grid_size, i*grid_size, grid_size, grid_size);
                if (!val.reduceLife()) { // smoke gone up in smoke
                    new_grid[i][j] = EMPTY;
                    continue;
                }
                if (i != 0) {
                    new_grid[i][j] = val;
                    const val_above = grid[i-1][j];
                    if (val_above instanceof Sand || val_above instanceof Empty) { // above is sand/empty
                        new_grid[i][j] = EMPTY;
                        new_grid[i-1][j] = val;
                    } else if (val_above instanceof Smoke) { // above is smoke
                        const variance = [-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5]
                        const dir = variance[Math.floor(Math.random()*variance.length)];
                        if (j+dir >= 0 && j+dir < cols && grid[i-1][j+dir] instanceof Empty) {
                            new_grid[i][j] = EMPTY;
                            new_grid[i-1][j+dir] = val;
                        }
                    }
                } else {
                    new_grid[i][j] = EMPTY;
                }
            } else if (val instanceof Fire)  {
                ctx.fillStyle = val.colour();
                ctx.fillRect(j*grid_size, i*grid_size, grid_size, grid_size);
                const smoke = new Smoke(Math.random()*10, 70 + Math.random()*30);
                if (!val.reduceLife()) {
                    new_grid[i][j] = smoke;
                    continue;
                }
                new_grid[i][j] = val;
                let breakk = false;
                for (let k = -1; k <= 1; k++) {
                    if (breakk) break;
                    for (let l = -1; l <=1; l++) {
                        if ((i+k)>=0 && (i+k)<rows && (j+l)>=0 && (j+l)<cols) {
                            if ((grid[i+k][j+l] instanceof Wood || grid[i+k][j+l] instanceof Sand) && Math.random() > 0.8) {
                                new_grid[i+k][j+l] = new Fire(Math.random()*10, 60 + Math.random()*20);
                                new_grid[i][j] = smoke;
                                breakk = true;
                                break;
                            } else if  (grid[i+k][j+l] instanceof Empty && Math.random() > 0.8) {
                                new_grid[i][j] = EMPTY;
                                new_grid[i+k][j+l] = val;
                                breakk = true;
                                break;
                            }
                        }
                    }
                }
            } 
        }
    }

    grid = new_grid;

    window.requestAnimationFrame(draw);
}

window.requestAnimationFrame(draw);


const mouse_grid_count = 3;
let mouse_grid = Array(mouse_grid_count).fill().map(() => Array(mouse_grid_count).fill(0));

function init_particles(coords, mouseDown) {
    for (let i = -mouse_grid_count/2; i <= mouse_grid_count/2; i++) {
        for (let j = -mouse_grid_count/2; j <= mouse_grid_count/2; j++) {
            let coords_2 = [Math.floor(coords[0] + i), Math.floor(coords[1] + j)]
            if (coords_2[0] >= 0 && coords_2[0] < rows && coords_2[1] >= 0 && coords_2[1] < cols) {

                switch (mode) {
                    case 1:
                        if (Math.random() > 0.3){
                            const sand = new Sand(Math.random()*10);
                            if (mouseDown)
                                grid[coords_2[0]][coords_2[1]] = sand;
                            ctx.fillStyle = sand.colour();
                            ctx.fillRect(coords_2[0], coords_2[1], grid_size, grid_size);
                        }
                        break;
                    case 2:
                        const wood = new Wood(Math.random()*10);
                        if (mouseDown)
                            grid[coords_2[0]][coords_2[1]] = wood;
                        ctx.fillStyle = wood.colour();
                        ctx.fillRect(coords_2[0], coords_2[1], grid_size, grid_size);
                        break;
                    case 3:
                        if (Math.random() > 0.5) {
                            const smoke = new Smoke(Math.random()*10, 70 + Math.random()*30);
                            if (mouseDown)
                                grid[coords_2[0]][coords_2[1]] = smoke;
                            ctx.fillStyle = smoke.colour();
                            ctx.fillRect(coords_2[0], coords_2[1], grid_size, grid_size);
                        }

                        break;
                    case 4:
                        if (Math.random() > 0.5) {
                            const fire = new Fire(Math.random()*10, 60 + Math.random()*20);
                            if (mouseDown)
                                grid[coords_2[0]][coords_2[1]] = fire;
                            ctx.fillStyle = fire.colour();
                            ctx.fillRect(coords_2[0], coords_2[1], grid_size, grid_size);
                        }

                        break;
                    default:
                        console.error(`Unknown mode ${mode}`);
                }
            }
        }
    }
}

canvas.addEventListener("mousedown", (e) => {
    let coords = [e.offsetY, e.offsetX];
    if (e.button == 0) {
        mouseDown = true;
        let coords = [e.offsetY / grid_size, e.offsetX / grid_size];
        init_particles(coords, mouseDown);
    }
})

let mouseDown = false

canvas.addEventListener("mouseup", (e) => {
    if (e.button == 0) {
        mouseDown = false;
    }
})

canvas.addEventListener("mousemove", (e) => {
    let coords = [e.offsetY / grid_size, e.offsetX / grid_size];
    init_particles(coords, mouseDown);
})

clear.addEventListener("click", () => {
    grid.map((a) => a.fill(new Empty()));
})

sand.addEventListener("click", () => {
    mode = 1;
})

wood.addEventListener("click", () => {
    mode = 2;
})

smoke.addEventListener("click", () => {
    mode = 3;
})

fire.addEventListener("click", () => {
    mode = 4;
})
