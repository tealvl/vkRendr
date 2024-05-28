#pragma once
#include <GLFW/glfw3.h>

namespace rendr {
    enum Key {

#define KEY(LITERAL) KEY_ ##LITERAL = #LITERAL[0] ,
        KEY(A)
        KEY(B)
        KEY(C)
        KEY(D)
        KEY(E)
        KEY(F)
        KEY(G)
        KEY(H)
        KEY(I)
        KEY(J)
        KEY(K)
        KEY(L)
        KEY(M)
        KEY(N)
        KEY(O)
        KEY(P)
        KEY(Q)
        KEY(R)
        KEY(S)
        KEY(T)
        KEY(U)
        KEY(V)
        KEY(W)
        KEY(X)
        KEY(Y)
        KEY(Z)
#undef KEY
#define KEY(NAME) KEY_ ##NAME
        KEY(SEMICOLON) = ';',
        KEY(QUOTE) = '\'',
        KEY(APOSTROPHE) = '`',
        KEY(L_BRACKET) = '[',
        KEY(R_BRACKET) = ']',
        KEY(COMMA) = ',',
        KEY(POINT) = '.',
        KEY(SLASH) = '/',
        KEY(BACKSLASH) = '\\',
#undef KEY
#define KEY(NUMERIC) KEY_NUM_ ##NUMERIC = NUMERIC ,
        KEY(0)
        KEY(1)
        KEY(2)
        KEY(3)
        KEY(4)
        KEY(5)
        KEY(6)
        KEY(7)
        KEY(8)
        KEY(9)
#undef KEY
#define KEY(NUM_PAD) KEY_NUM_PAD_ ##NUM_PAD,
        KEY(0)
        KEY(1)
        KEY(2)
        KEY(3)
        KEY(4)
        KEY(5)
        KEY(6)
        KEY(7)
        KEY(8)
        KEY(9)
#undef KEY
#define KEY(FUNC) KEY_F ##FUNC,
        KEY(1)
        KEY(2)
        KEY(3)
        KEY(4)
        KEY(5)
        KEY(6)
        KEY(7)
        KEY(8)
        KEY(9)
        KEY(10)
        KEY(11)
        KEY(12)
#undef KEY
#define KEY(NAME) KEY_ ##NAME
        KEY(TAB),
        KEY(CAPS_LOCK),
        KEY(LEFT_SHIFT),
        KEY(RIGHT_SHIFT),
        KEY(LEFT_CONTROL),
        KEY(RIGHT_CONTROL),
        KEY(LEFT_ALT),
        KEY(RIGHT_ALT),
        KEY(SPACE),
        KEY(ENTER),
        KEY(BACKSPACE),
        KEY(INSERT),
        KEY(DELETE),
        KEY(HOME),
        KEY(END),
        KEY(PAGE_UP),
        KEY(PAGE_DOWN),
        KEY(NUM_PAD_ADD),
        KEY(NUM_PAD_SUBTRACT),
        KEY(NUM_PAD_DIVIDE),
        KEY(NUM_PAD_MULTIPLY),
        KEY(NUM_PAD_ENTER),
        KEY(NUM_PAD_DECIMAL),
        KEY(NUM_PAD_EQUAL),
        KEY(MINUS),
        KEY(EQUAL),
        KEY(ESCAPE),
        KEY(LEFT),
        KEY(RIGHT),
        KEY(UP),
        KEY(DOWN),
        KEY(UNKNOWN),
    };
#undef KEY
   static_assert(KEY_V == static_cast<Key>('V'));


static int glfwKeyTranslate(int key){
        switch (key) {
#define TRANSLATE_KEY(KEY) case GLFW_KEY_ ##KEY: return KEY_ ##KEY;
            TRANSLATE_KEY(A)
            TRANSLATE_KEY(B)
            TRANSLATE_KEY(C)
            TRANSLATE_KEY(D)
            TRANSLATE_KEY(E)
            TRANSLATE_KEY(F)
            TRANSLATE_KEY(G)
            TRANSLATE_KEY(H)
            TRANSLATE_KEY(I)
            TRANSLATE_KEY(J)
            TRANSLATE_KEY(K)
            TRANSLATE_KEY(L)
            TRANSLATE_KEY(M)
            TRANSLATE_KEY(N)
            TRANSLATE_KEY(O)
            TRANSLATE_KEY(P)
            TRANSLATE_KEY(Q)
            TRANSLATE_KEY(R)
            TRANSLATE_KEY(S)
            TRANSLATE_KEY(T)
            TRANSLATE_KEY(U)
            TRANSLATE_KEY(V)
            TRANSLATE_KEY(W)
            TRANSLATE_KEY(X)
            TRANSLATE_KEY(Y)
            TRANSLATE_KEY(Z)
            TRANSLATE_KEY(SEMICOLON)
            TRANSLATE_KEY(SLASH)
            TRANSLATE_KEY(COMMA)
            TRANSLATE_KEY(BACKSLASH)
            TRANSLATE_KEY(APOSTROPHE)
            TRANSLATE_KEY(SPACE)
            TRANSLATE_KEY(BACKSPACE)
            TRANSLATE_KEY(ESCAPE)
            case GLFW_KEY_RIGHT_BRACKET: return KEY_R_BRACKET;
            case GLFW_KEY_LEFT_BRACKET: return KEY_L_BRACKET;
            TRANSLATE_KEY(F1)
            TRANSLATE_KEY(F2)
            TRANSLATE_KEY(F3)
            TRANSLATE_KEY(F4)
            TRANSLATE_KEY(F5)
            TRANSLATE_KEY(F6)
            TRANSLATE_KEY(F7)
            TRANSLATE_KEY(F8)
            TRANSLATE_KEY(F9)
            TRANSLATE_KEY(F10)
            TRANSLATE_KEY(F11)
            TRANSLATE_KEY(RIGHT)
            TRANSLATE_KEY(LEFT)
            TRANSLATE_KEY(UP)
            TRANSLATE_KEY(DOWN)
            TRANSLATE_KEY(HOME)
            TRANSLATE_KEY(END)
            TRANSLATE_KEY(INSERT)
            TRANSLATE_KEY(DELETE)
            TRANSLATE_KEY(ENTER)
            TRANSLATE_KEY(LEFT_SHIFT)
            TRANSLATE_KEY(RIGHT_SHIFT)
            TRANSLATE_KEY(LEFT_CONTROL)
            TRANSLATE_KEY(RIGHT_CONTROL)
            TRANSLATE_KEY(LEFT_ALT)
            TRANSLATE_KEY(RIGHT_ALT)
#undef TRANSLATE_KEY
#define TRANSLATE_KEY(KEY) case GLFW_KEY_ ##KEY: return KEY_NUM_ ##KEY;
            TRANSLATE_KEY(1)
            TRANSLATE_KEY(2)
            TRANSLATE_KEY(3)
            TRANSLATE_KEY(4)
            TRANSLATE_KEY(5)
            TRANSLATE_KEY(6)
            TRANSLATE_KEY(7)
            TRANSLATE_KEY(8)
            TRANSLATE_KEY(9)
#undef TRANSLATE_KEY
#define TRANSLATE_KEY(KEY) case GLFW_KEY_KP_ ##KEY: return KEY_NUM_PAD_ ##KEY;
            TRANSLATE_KEY(1)
            TRANSLATE_KEY(2)
            TRANSLATE_KEY(3)
            TRANSLATE_KEY(4)
            TRANSLATE_KEY(5)
            TRANSLATE_KEY(6)
            TRANSLATE_KEY(7)
            TRANSLATE_KEY(8)
            TRANSLATE_KEY(9)
            TRANSLATE_KEY(ADD)
            TRANSLATE_KEY(SUBTRACT)
            TRANSLATE_KEY(DIVIDE)
            TRANSLATE_KEY(MULTIPLY)
            TRANSLATE_KEY(ENTER)
            TRANSLATE_KEY(DECIMAL)
            TRANSLATE_KEY(EQUAL)
            default: return KEY_UNKNOWN;
        }
    }
}



