package main

/*
#cgo LDFLAGS: -lSDL2 -lSDL2_ttf
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

Uint32 sdl_event_type(SDL_Event* e) {
    return e->type;
}
*/
import "C"

import (
	"fmt"
	"math"
	"math/rand"
	"os"
	"runtime"
	"time"
	"unsafe"
)

const (
	ScreenWidth   = 800
	ScreenHeight  = 600
	FPS           = 60
	FrameTime     = time.Second / FPS
	BallRadius    = 8 // i know the ball is a square so radius=width/2 or height/2
	BallSpeed     = 700.0
	MinBallXSpeed = 0.3 // (v.x/v)
	PaddleWidth   = 12
	PaddleHeight  = 100
	PaddleSpeed   = 850
	FontSize      = 20
	DividerWidth  = 16
	GameResetTime = 1.5
	WinScore      = 3
)

type Vector2f struct {
	x, y float32
}

func (v *Vector2f) normalize() {
	// if v.x < MinBallXSpeed {
	// 	v.x = MinBallXSpeed
	// }
	d := float32(math.Sqrt(float64(v.x*v.x + v.y*v.y)))
	if d == 0 {
		return
	}
	v.x /= d
	v.y /= d
}

type Screen int

const (
	ScreenStart Screen = iota
	ScreenGame
	ScreenFinish
)

var gameState = struct {
	ballPos       Vector2f
	ballDir       Vector2f
	lPaddlePos    float32
	rPaddlePos    float32
	scoreL        int
	scoreR        int
	gameResetTime float32
	aiMode        bool
	screen        Screen
	winner        int
}{
	ballPos:       Vector2f{ScreenWidth / 2, ScreenHeight / 2},
	ballDir:       Vector2f{0, 0},
	lPaddlePos:    ScreenHeight / 2,
	rPaddlePos:    ScreenHeight / 2,
	scoreL:        0,
	scoreR:        0,
	gameResetTime: GameResetTime,
	aiMode:        false,
	screen:        ScreenStart,
	winner:        0,
}

var (
	ttfFont    *C.TTF_Font
	fontColour C.SDL_Color
)

type cachedText struct {
	text string
	tex  *C.SDL_Texture
}

func (c *cachedText) destroy() {
	if c.tex != nil {
		C.SDL_DestroyTexture(c.tex)
		c.tex = nil
	}
}

func makeTextTexture(renderer *C.SDL_Renderer, text string) *C.SDL_Texture {
	cText := C.CString(text)
	defer C.free(unsafe.Pointer(cText))

	surf := C.TTF_RenderText_Solid(ttfFont, cText, fontColour)
	if surf == nil {
		fmt.Fprintf(os.Stderr, "Cannot create surface %s\n", C.GoString(C.TTF_GetError()))
		return nil
	}
	defer C.SDL_FreeSurface(surf)

	tex := C.SDL_CreateTextureFromSurface(renderer, surf)
	if tex == nil {
		fmt.Fprintf(os.Stderr, "Cannot create texture %s\n", C.GoString(C.SDL_GetError()))
		return nil
	}
	return tex
}

func (c *cachedText) ensure(renderer *C.SDL_Renderer, text string) {
	if c.tex != nil && c.text == text {
		return
	}
	c.destroy()
	c.text = text
	c.tex = makeTextTexture(renderer, text)
}

var (
	textStart    = "Start? (Press s)"
	textStartPos = C.SDL_Rect{
		C.int(ScreenWidth/2 - len(textStart)*30/2),
		ScreenHeight / 2,
		C.int(len(textStart) * 30),
		40,
	}
	textRestart    = "Restart? (Press r)"
	textRestartPos = C.SDL_Rect{
		C.int(ScreenWidth/2 - len(textRestart)*15/2),
		5 * ScreenHeight / 8,
		C.int(len(textRestart) * 15),
		20,
	}
	textQuit    = "Quit? (Press q)"
	textQuitPos = C.SDL_Rect{
		C.int(ScreenWidth/2 - len(textQuit)*15/2),
		7 * ScreenHeight / 8,
		C.int(len(textQuit) * 15),
		20,
	}
	textWinPos = C.SDL_Rect{ScreenWidth/2 - 7*20, ScreenHeight / 8, 14 * 20, 30}
	lTextPos   = C.SDL_Rect{ScreenWidth / 4, ScreenHeight / 10, 20, 30}
	rTextPos   = C.SDL_Rect{3 * ScreenWidth / 4, ScreenHeight / 10, 20, 30}
)

func isKeyPressed(keys *C.Uint8, scancode C.SDL_Scancode) bool {
	return *(*C.Uint8)(unsafe.Add(unsafe.Pointer(keys), scancode)) != 0
}

func populateRandInitBall() {
	gameState.ballPos = Vector2f{ScreenWidth / 2, (ScreenHeight / 2) * (rand.Float32() + 0.5)}

	angle := rand.Float64()*60 - 30 // -30 to +30 degrees

	if rand.Intn(2) == 0 {
		angle += 180
	}

	rad := angle * math.Pi / 180

	gameState.ballDir.x = float32(math.Cos(rad))
	gameState.ballDir.y = float32(math.Sin(rad))
}

func resetGame() {
	gameState.lPaddlePos = ScreenHeight / 2
	gameState.rPaddlePos = ScreenHeight / 2

	populateRandInitBall()

	gameState.scoreL = 0
	gameState.scoreR = 0
	gameState.winner = 0
	gameState.screen = ScreenGame
}

func updateGame(dt float32, lMotion, rMotion int) {
	if gameState.screen != ScreenGame {
		return
	}

	// update paddlePos
	newLPos := gameState.lPaddlePos
	newRPos := gameState.rPaddlePos
	newLPos += float32(lMotion) * dt * PaddleSpeed
	newRPos += float32(rMotion) * dt * PaddleSpeed

	if newLPos >= PaddleHeight/2 && newLPos < (ScreenHeight-PaddleHeight/2) {
		gameState.lPaddlePos = newLPos
	}
	if newRPos >= PaddleHeight/2 && newRPos < (ScreenHeight-PaddleHeight/2) {
		gameState.rPaddlePos = newRPos
	}

	if gameState.gameResetTime > 0 {
		gameState.gameResetTime -= dt
		if gameState.gameResetTime < 0 {
			gameState.gameResetTime = 0
		}

		return
	}

	// update ball pos
	gameState.ballPos.x += gameState.ballDir.x * BallSpeed * dt
	gameState.ballPos.y += gameState.ballDir.y * BallSpeed * dt

	ballPosx := gameState.ballPos.x
	ballPosy := gameState.ballPos.y

	// reflect from paddle
	if ballPosx <= 2*PaddleWidth+BallRadius {
		// might reflect from l
		if ballPosy > (gameState.lPaddlePos-PaddleHeight/2-BallRadius) &&
			ballPosy < (gameState.lPaddlePos+PaddleHeight/2+BallRadius) { // hit on side
			gameState.ballDir.x *= -1
			gameState.ballDir.y += float32(lMotion) * 0.2
			gameState.ballDir.normalize()
			gameState.ballPos.x = 2*PaddleWidth + BallRadius
		} else if ballPosy == (gameState.lPaddlePos-PaddleHeight/2-BallRadius) ||
			ballPosy == (gameState.lPaddlePos+PaddleHeight/2+BallRadius) { // hit on top/bottom
			gameState.ballDir.y *= -1
			gameState.ballDir.y += float32(lMotion) * 0.5
			gameState.ballDir.normalize()
			gameState.ballPos.x = 2*PaddleWidth + BallRadius
		}
	} else if ballPosx >= (ScreenWidth - 2*PaddleWidth - BallRadius) {
		// might reflect from r
		if ballPosy > (gameState.rPaddlePos-PaddleHeight/2-BallRadius) &&
			ballPosy < (gameState.rPaddlePos+PaddleHeight/2+BallRadius) { // hit on side
			gameState.ballDir.x *= -1
			gameState.ballDir.y += float32(rMotion) * 0.2
			gameState.ballDir.normalize()
			gameState.ballPos.x = ScreenWidth - 2*PaddleWidth - BallRadius
		} else if ballPosy == (gameState.rPaddlePos-PaddleHeight/2-BallRadius) ||
			ballPosy == (gameState.rPaddlePos+PaddleHeight/2+BallRadius) { // hit on top/bottom
			gameState.ballDir.y *= -1
			gameState.ballDir.y += float32(rMotion) * 0.5
			gameState.ballDir.normalize()
			gameState.ballPos.x = ScreenWidth - 2*PaddleWidth - BallRadius
		}
	}

	//	reflect from top,bottom
	if gameState.ballPos.y < BallRadius {
		gameState.ballDir.y = -gameState.ballDir.y
		gameState.ballPos.y = BallRadius
	} else if gameState.ballPos.y > (ScreenHeight - BallRadius) {
		gameState.ballDir.y = -gameState.ballDir.y
		gameState.ballPos.y = ScreenHeight - BallRadius
	}

	// score change when out of screen
	if gameState.ballPos.x < BallRadius {
		gameState.scoreR += 1
		gameState.gameResetTime = GameResetTime
		populateRandInitBall()
	} else if gameState.ballPos.x > (ScreenWidth - BallRadius) {
		gameState.scoreL += 1
		gameState.gameResetTime = GameResetTime
		populateRandInitBall()
	}
	if gameState.scoreL >= WinScore {
		gameState.screen = ScreenFinish
		gameState.winner = -1
	}
	if gameState.scoreR >= WinScore {
		gameState.screen = ScreenFinish
		gameState.winner = 1
	}
}

var (
	dividerRects []C.SDL_Rect
	rects        = make([]C.SDL_Rect, 0, 64)
)

func renderGame(renderer *C.SDL_Renderer) {
	if gameState.screen != ScreenGame {
		return
	}
	rects = rects[:0]
	rects = append(rects, dividerRects...)

	// Left, Right paddle
	lPaddlePos := int(gameState.lPaddlePos)
	rPaddlePos := int(gameState.rPaddlePos)
	lPaddleRect := C.SDL_Rect{
		C.int(PaddleWidth),
		C.int(lPaddlePos - PaddleHeight/2),
		PaddleWidth,
		PaddleHeight,
	}
	rPaddleRect := C.SDL_Rect{
		C.int(ScreenWidth - PaddleWidth),
		C.int(rPaddlePos - PaddleHeight/2),
		PaddleWidth,
		PaddleHeight,
	}
	rects = append(rects, lPaddleRect)
	rects = append(rects, rPaddleRect)

	// ball
	ballX := int(gameState.ballPos.x)
	ballY := int(gameState.ballPos.y)
	ballRect := C.SDL_Rect{
		C.int(ballX - BallRadius),
		C.int(ballY - BallRadius),
		BallRadius * 2,
		BallRadius * 2,
	}
	rects = append(rects, ballRect)

	C.SDL_RenderFillRects(renderer, &rects[0], C.int(len(rects)))
}

func renderText(renderer *C.SDL_Renderer, startText, restartText, quitText, winText *cachedText, lScoreText, rScoreText *cachedText) {
	switch gameState.screen {
	case ScreenStart:
		if startText.tex != nil {
			C.SDL_RenderCopy(renderer, startText.tex, nil, &textStartPos)
		}

	case ScreenGame:
		lScoreText.ensure(renderer, fmt.Sprintf("%d", gameState.scoreL))
		rScoreText.ensure(renderer, fmt.Sprintf("%d", gameState.scoreR))
		if lScoreText.tex != nil {
			C.SDL_RenderCopy(renderer, lScoreText.tex, nil, &lTextPos)
		}
		if rScoreText.tex != nil {
			C.SDL_RenderCopy(renderer, rScoreText.tex, nil, &rTextPos)
		}

	case ScreenFinish:
		var win string
		if gameState.winner > 0 {
			win = "Player 2 wins!"
		} else {
			win = "Player 1 wins!"
		}
		winText.ensure(renderer, win)
		if winText.tex != nil {
			C.SDL_RenderCopy(renderer, winText.tex, nil, &textWinPos)
		}
		if restartText.tex != nil {
			C.SDL_RenderCopy(renderer, restartText.tex, nil, &textRestartPos)
		}
		if quitText.tex != nil {
			C.SDL_RenderCopy(renderer, quitText.tex, nil, &textQuitPos)
		}

	default:
		panic("unreachable gameState.screen\n")
	}
}

func main() {
	// https://groups.google.com/g/golang-nuts/c/IiWZ2hUuLDA/m/SNKYYZBelsYJ
	runtime.LockOSThread()

	if C.SDL_Init(C.SDL_INIT_VIDEO) != 0 {
		fmt.Fprintf(os.Stderr, "SDL_Init failed: %s\n", C.GoString(C.SDL_GetError()))
		os.Exit(1)
	}
	defer C.SDL_Quit()

	if C.TTF_Init() != 0 {
		fmt.Fprintf(os.Stderr, "TTF_Init failed: %s\n", C.GoString(C.TTF_GetError()))
		os.Exit(1)
	}
	defer C.TTF_Quit()

	fontFile := "/usr/share/fonts/TTF/InconsolataNerdFontPropo-Regular.ttf"
	cFontFile := C.CString(fontFile)
	ttfFont = C.TTF_OpenFont(cFontFile, 32)
	C.free(unsafe.Pointer(cFontFile))
	if ttfFont == nil {
		fmt.Fprintf(os.Stderr, "Cannot open font file %s: %s\n", fontFile, C.GoString(C.TTF_GetError()))
		os.Exit(1)
	}
	defer C.TTF_CloseFont(ttfFont)
	fontColour = C.SDL_Color{255, 255, 255, 255}

	title := C.CString("Pong SDL")
	window := C.SDL_CreateWindow(
		title,
		C.SDL_WINDOWPOS_CENTERED,
		C.SDL_WINDOWPOS_CENTERED,
		ScreenWidth,
		ScreenHeight,
		0,
	)
	defer C.free(unsafe.Pointer(title))
	if window == nil {
		fmt.Fprintf(os.Stderr, "SDL_CreateWindow failed: %s\n", C.GoString(C.SDL_GetError()))
		os.Exit(1)
	}
	defer C.SDL_DestroyWindow(window)

	renderer := C.SDL_CreateRenderer(window, -1, C.SDL_RENDERER_ACCELERATED)
	if renderer == nil {
		fmt.Fprintf(os.Stderr, "SDL_CreateRenderer failed: %s\n", C.GoString(C.SDL_GetError()))
		os.Exit(1)
	}
	defer C.SDL_DestroyRenderer(renderer)

	dividerRects = dividerRects[:0]
	divider := true
	for i := 0; i < ScreenHeight-DividerWidth; i += DividerWidth {
		if divider {
			dividerRects = append(dividerRects, C.SDL_Rect{
				C.int(ScreenWidth/2 - DividerWidth/2),
				C.int(i),
				DividerWidth,
				DividerWidth,
			})
		}
		divider = !divider
	}

	startText := &cachedText{text: textStart}
	startText.tex = makeTextTexture(renderer, textStart)
	defer startText.destroy()

	restartText := &cachedText{text: textRestart}
	restartText.tex = makeTextTexture(renderer, textRestart)
	defer restartText.destroy()

	quitText := &cachedText{text: textQuit}
	quitText.tex = makeTextTexture(renderer, textQuit)
	defer quitText.destroy()

	winText := &cachedText{}
	defer winText.destroy()

	lScoreText := &cachedText{}
	defer lScoreText.destroy()

	rScoreText := &cachedText{}
	defer rScoreText.destroy()

	populateRandInitBall()

	var ev C.SDL_Event
	quit := false

	last := time.Now()
	for !quit {
		now := time.Now()
		dt := now.Sub(last).Seconds()
		last = now
		lMotion, rMotion := 0, 0

		if gameState.screen == ScreenGame {
			for C.SDL_PollEvent(&ev) != 0 {
				switch C.sdl_event_type(&ev) {
				case C.SDL_QUIT:
					quit = true
				}
			}
		} else {
			if C.SDL_WaitEventTimeout(&ev, 100) != 0 {
				switch C.sdl_event_type(&ev) {
				case C.SDL_QUIT:
					quit = true
				}
				for C.SDL_PollEvent(&ev) != 0 {
					switch C.sdl_event_type(&ev) {
					case C.SDL_QUIT:
						quit = true
					}
				}
			}
		}

		keys := C.SDL_GetKeyboardState(nil)
		switch gameState.screen {
		case ScreenStart:
			if isKeyPressed(keys, C.SDL_SCANCODE_S) {
				gameState.screen = ScreenGame
			}
		case ScreenGame:
			if isKeyPressed(keys, C.SDL_SCANCODE_W) {
				lMotion = -1
			}
			if isKeyPressed(keys, C.SDL_SCANCODE_S) {
				lMotion = 1
			}
			if isKeyPressed(keys, C.SDL_SCANCODE_UP) {
				rMotion = -1
			}
			if isKeyPressed(keys, C.SDL_SCANCODE_DOWN) {
				rMotion = 1
			}
		case ScreenFinish:
			if isKeyPressed(keys, C.SDL_SCANCODE_Q) {
				quit = true
			}
			if isKeyPressed(keys, C.SDL_SCANCODE_R) {
				resetGame()
			}
		default:
			panic("unreachable gameState.screen\n")
		}

		updateGame(float32(dt), lMotion, rMotion)

		C.SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0)
		C.SDL_RenderClear(renderer)

		C.SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255)
		renderGame(renderer)
		renderText(renderer, startText, restartText, quitText, winText, lScoreText, rScoreText)

		C.SDL_RenderPresent(renderer)

		if gameState.screen == ScreenGame {
			sleep := FrameTime - time.Since(now)
			if sleep > 0 {
				time.Sleep(sleep)
			}
		}
	}
}
