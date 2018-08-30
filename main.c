/* http://www.inf.pucrs.br/~manssour/Allegro/#eventos */

#include "utils.h"
#include "render.h"
#include "ambiente.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef enum op
{
    HOWPLAY = 1, PLAY, EXIT
} OPCAO;

enum MYKEYS {
    KEY_LEFT=0, KEY_RIGHT, KEY_UP
};


ALLEGRO_FONT* fnt = NULL;
ALLEGRO_FONT* fnt_score = NULL;
ALLEGRO_EVENT_QUEUE* fila = NULL;

//static ALLEGRO_THREAD* threads[NTHREADS];
static ALLEGRO_THREAD* thr_eventos = NULL;
static ALLEGRO_THREAD* thr_colisao = NULL;
static ALLEGRO_THREAD* thr_render = NULL;
BITMAP* bmp;

float x, y, angle;
bool stop;

long int record;
short nlifes = 10;
Pixel borda[8];

short pisca;
bool pressed_keys[3] = {false, false, false};

double time_init_frame;
int fps;


int menu();
void play();
void finalizaJogo();
void inicializaJogo();
void inicializaAllegro();
void trata_evento_tecla_direcao();
void trata_evento_tecla_velocidade();
void espera_tempo_frame();
void monta_frame_final();
void print_pause_frame();
void limpa_buffer_teclado (int nit);
int detectaColisao ();
void ajusta_barco();
void initialize_time_init_frame();
double get_time_after_frame();
void load_variables_global();
int teste_oito_vizinhos (Pixel centro);
Pixel rotacao (Pixel p, float angle);
int ehMargem(Pixel p);


void* thread_eventos(ALLEGRO_THREAD* thread, void* arg);
void* thread_colisao(ALLEGRO_THREAD* thread, void* arg);



int main()
{
    inicializaAllegro();

    load_variables_global();
    load_bitmaps();
    inicializaJogo();
    
    al_set_target_backbuffer(screen);

    mutex = al_create_mutex();

    thr_eventos = al_create_thread(thread_eventos, NULL);
    //thr_colisao = al_create_thread(thread_colisao, NULL);
    thr_render = al_create_thread(thread_render_cenarios, NULL);

    al_start_thread(thr_eventos);
    //al_start_thread(thr_colisao);
    al_start_thread(thr_render);

    printf("Rodando as threads ......\n");
    al_rest(0.5);
    
    al_lock_mutex(mutex);
    ptr_frame = head_bmp->prox;
    al_unlock_mutex(mutex);
    
    ptr_frame->x = X_INICIAL_BARCO;
    ptr_frame->angle = 0;
    x = X_INICIAL_BARCO;
    y = Y_INICIAL_BARCO;
    play();
    
    al_join_thread(thr_eventos, NULL);
    //al_join_thread(thr_colisao, NULL);
    al_join_thread(thr_render, NULL);
    al_destroy_thread(thr_eventos);
    //al_destroy_thread(thr_colisao);
    al_destroy_thread(thr_render);

    al_destroy_mutex(mutex);
    
    return 0;
}

void* thread_colisao(ALLEGRO_THREAD* thread, void* arg) {
    al_set_target_backbuffer(NULL);
    
    struct timeval tm_colisao;
    // inicio da contagem da primeira colisão
    gettimeofday(&tm_colisao, NULL);

    while (!fim) 
    {
        if (detectaColisao() && relogio(tm_colisao) > 0.2) 
        {
            pisca = 5;
            
            nlifes--;

            if (nlifes < 0)
                nlifes = 0;
            
            // inicio da contagem da proxima colisão
            gettimeofday(&tm_colisao, NULL);

            //al_rest(4);
            //fim = true;
        }
    }

    return NULL;
}


void* thread_eventos(ALLEGRO_THREAD* thread, void* arg) {
    al_set_target_backbuffer(NULL);
    ALLEGRO_EVENT event;

    while (1) 
    {
        if (!al_is_event_queue_empty(fila)) 
        {
            al_get_next_event(fila, &event);

            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                fim = true;
                break;
            }

            // tecla ENTER foi pressionada 
            if (event.type == ALLEGRO_EVENT_KEY_DOWN && 
                event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                
                stop = !stop;
                pressed_keys[KEY_LEFT] = false;
                pressed_keys[KEY_RIGHT] = false;
                pressed_keys[KEY_UP] = false;
                fim = false;
            }
        }

        if (!stop) 
        {
            if (event.type == ALLEGRO_EVENT_KEY_UP) 
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
                    pressed_keys[KEY_LEFT] = false;
                
                else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                    pressed_keys[KEY_RIGHT] = false;

                else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                    pressed_keys[KEY_UP] = false;
            }
                
            else if (event.type == ALLEGRO_EVENT_KEY_DOWN) 
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
                    pressed_keys[KEY_LEFT] = true;
                
                else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                    pressed_keys[KEY_RIGHT] = true;

                else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
                    pressed_keys[KEY_UP] = true;
                }
            }

            //trata_evento_tecla_velocidade();
        }
    }

    printf("saiu3\n");

    return NULL;
}

void play()
{
    //ALLEGRO_EVENT event;
    //struct timeval tm_colisao;

    // inicio da contagem da primeira colisão
    //gettimeofday(&tm_colisao, NULL);
    
    //monta_frame_final();
    
    struct timeval tm_colisao;
    // inicio da contagem da primeira colisão
    gettimeofday(&tm_colisao, NULL);

    printf("BY = %d\n", DISPLAY_HIGHT/NROWS);
    printf("BX = %d\n", DISPLAY_WEIGHT/NCOLS);

    while (!fim)
    {

        // if fila nao vazia da thread viria aqui

        if (!stop)
        {   
            // if ev.type da thread viria aqui         
            
            //atualizaRio viria aqui;                

            //espera_tempo_frame();    
            al_lock_mutex(mutex);
            bmp = get_next_bmp();
            al_unlock_mutex(mutex);

            monta_frame_final();
            
            al_flip_display();

            trata_evento_tecla_direcao();           
            
            
            if (detectaColisao() && relogio(tm_colisao) > 0.15) 
            {
                pisca = 5;
                
                nlifes--;

                if (nlifes < 0)
                    nlifes = 0;
                
                // inicio da contagem da proxima colisão
                gettimeofday(&tm_colisao, NULL);
            }
            

            //ptr_frame = ptr_frame->prox;
            al_lock_mutex(mutex);
            remove_bmp();
            al_unlock_mutex(mutex);
            
        
            al_rest(0.0);
        }
            
        else {
            print_pause_frame();
            
            while (!fim && stop) al_rest(0.15);
        }
    }

    finalizaJogo();
    printf("saiu1\n");         
}

void monta_frame_final() {
    //al_clear_to_color(al_map_rgb(cor.RED, cor.GREEN, cor.BLUE));
    char score_str[30];
    char life_str[5];
    Node* node;

    if (!bmp) {
        fprintf(stderr, "Socorro! BUFFER de BITMAPs vazio!!!\n");
        exit(0);
    }

    al_draw_bitmap(bmp->bitmap, 0, 0, 0);

    if (!pisca) {
        al_draw_rotated_bitmap(barco, w/2, h/2, x, Y_INICIAL_BARCO, angle, 0);
        //al_draw_line(x+w/2, 0, x+w/2, DISPLAY_HIGHT, al_map_rgb(0,0,0), 1);
    }

    else {
        //al_draw_tinted_bitmap(bitmap, al_map_rgba_f(1, 1, 1, 0.5), x, y, 0);
        al_draw_tinted_rotated_bitmap(barco, al_map_rgba_f(1, 1, 1, 0.5),
            w/2, h/2, x, Y_INICIAL_BARCO, angle, 0);
        
        pisca--;
        //usleep(100500);
    }

    al_draw_filled_rectangle(0, DISPLAY_HIGHT-25, DISPLAY_WEIGHT, DISPLAY_HIGHT, al_map_rgb(0,0,0));

    sprintf(score_str, "score : %ld", score);
    score++;

    if (score > 1000000)
        score = 1000000;

    al_draw_text(fnt_score, al_map_rgb(255, 255, 255), DISPLAY_WEIGHT-50, DISPLAY_HIGHT-25, ALLEGRO_ALIGN_RIGHT, score_str);
    al_draw_text(fnt_score, al_map_rgb(255, 255, 255), 70, DISPLAY_HIGHT-25, ALLEGRO_ALIGN_RIGHT, "record :");

    sprintf(life_str, "%d", nlifes);
    
    al_draw_text(fnt_score, al_map_rgb(255, 255, 255), DISPLAY_WEIGHT/2-10, DISPLAY_HIGHT-25, ALLEGRO_ALIGN_RIGHT, life_str);

    al_draw_bitmap(al_load_bitmap("images/life.png"), DISPLAY_WEIGHT/2, DISPLAY_HIGHT-25, 0);
    
    //initialize_time_init_frame();
}


void load_variables_global() {
    stop = false;
    fps = FPS;
    //x = 200, y = (DISPLAY_HIGHT * 4)/5;
    //angle = 0;

    // cantos do barco
    borda[0] = (Pixel) {0, 0 - h/2};
    borda[1] = (Pixel) {0, 0 + h/2};    
    borda[2] = (Pixel) {0 - w/2, 0};       
    borda[3] = (Pixel) {0 + w/2, 0};
}


int detectaColisao () {
    //ALLEGRO_BITMAP* bmp = al_get_backbuffer(screen);
    Pixel p;
    short i;

    for (i = 0; i < 4; i++) {
        p = rotacao(borda[i], angle);
        
        //al_draw_circle(p.x, p.y, 1, al_map_rgb(0,0,0), 1);
        
        if (teste_oito_vizinhos(p)) {
            return 1;
        }
    }

    return 0;
}

int teste_oito_vizinhos (Pixel centro) {    
    // rgb da areia = (255, 230, 128)
    Pixel v;

    v.x = centro.x - 1, v.y = centro.y - 1;
    if (ehMargem(v)) { return 1;}

    v.x = centro.x - 1, v.y = centro.y;
    if (ehMargem(v)) { return 1;}
    
    v.x = centro.x - 1, v.y = centro.y + 1;
    if (ehMargem(v)) { return 1;}

    v.x = centro.x, v.y = centro.y - 1;
    if (ehMargem(v)) { return 1;}

    v.x = centro.x, v.y = centro.y + 1;
    if (ehMargem(v)) { return 1;}

    v.x = centro.x + 1, v.y = centro.y - 1;
    if (ehMargem(v)) { return 1;}

    v.x = centro.x + 1, v.y = centro.y;
    if (ehMargem(v)) { return 1;}

    v.x = centro.x + 1, v.y = centro.y + 1;
    if (ehMargem(v)) { return 1;}

    v.x = centro.x, v.y = centro.y;
    if (ehMargem(v)) { return 1;}

    return 0;
}

int ehMargem(Pixel p) {
    int linha = ceil((1.0 * p.y)/BLOCO_Y);
    int i;
    //Node* node = head;

    if (p.x <= MARGEM_ESQ * BLOCO_X || p.x >= MARGEM_DIR * BLOCO_X)
        return 1;

    //for (i = 0; i < linha; i++)
    //    node = node->prox;
    Node node = bmp->river_map[linha-1];
    
    int x = node.margem_esq * BLOCO_X;

    if (p.x <= x) return 1;

    x = node.margem_dir * BLOCO_X;

    if (p.x >= x) return 1;

    // testa se eh ilha
    
    if (node.inicio_ilha != -1) {
        x = node.inicio_ilha * BLOCO_X;

        if (p.x >= x-5 && p.x <= x + w_ilha+5 ) {
            //al_draw_bitmap(ilhabmp, x, ceil(p.y)-100, 0);
            
            return 1;
        }
    }
    
    return 0;
}


// em ralação a origem do plano xoy
// matriz de rotação:
// M = [cos(beta) -sen(-beta) ]
//     [sen(-beta)  cos(beta) ]
Pixel rotacao (Pixel p, float angle) {
    Pixel p2;
    float a, b;

    a = cos(angle);
    b = sin(angle);

    p2.x = x + (int) (a * p.x - b * p.y);
    p2.y = y + (int) (b * p.x + a * p.y);

    return p2;
}


double get_time_after_frame() {
    return al_get_time() - time_init_frame;
}

void initialize_time_init_frame() {
    time_init_frame = al_get_time();    
}

void espera_tempo_frame() {
    if (get_time_after_frame() < (1.0/fps)) {
        al_rest(fabs((1.0/fps) - get_time_after_frame()));
    }
}

void trata_evento_tecla_velocidade() {
    if (pressed_keys[KEY_UP] && fps < 140) {
        fps += 20;
    }

    else if (!pressed_keys[KEY_UP] && fps > FPS) {
        fps = FPS;
    }
}

void trata_evento_tecla_direcao() {
    if (pressed_keys[KEY_LEFT] ^ pressed_keys[KEY_RIGHT]  ) 
    {
        if (pressed_keys[KEY_LEFT]) 
        {
            x -= 10;

            if (angle > -ALLEGRO_PI/9)
            {
                // 10 graus
                angle -= ALLEGRO_PI/18;
            }
        }

        if (pressed_keys[KEY_RIGHT]) 
        {
            x += 10;

            if (angle < ALLEGRO_PI/9)
            {
                angle += ALLEGRO_PI/18;
            }
        }
    } else {
        ajusta_barco();
    }

    ptr_frame->x = x;
    ptr_frame->angle = angle;

}

void inicializaJogo () {
    screen = al_create_display(DISPLAY_WEIGHT, DISPLAY_HIGHT);
    al_set_window_position(screen, 300, 100);
    al_draw_bitmap(al_load_bitmap("images/boattrace.png"), 0, 0, 0);

    al_set_display_icon(screen, icon);
    al_set_window_title(screen, "River");
    al_flip_display();

    al_rest(0.3);
    
    al_resize_display(screen, DISPLAY_WEIGHT, DISPLAY_HIGHT);
    al_set_window_position(screen, 400, 50);

    if(!screen)
    {
        fprintf(stderr, "Falha ao criar a janela\n");
        return ;
    }

    //al_clear_to_color(al_map_rgb(cor.RED, cor.GREEN, cor.BLUE));
    
    fila = al_create_event_queue();
        
    al_register_event_source(fila, al_get_display_event_source(screen));
    al_register_event_source(fila, al_get_keyboard_event_source());


    if (!fila)
    {
        fprintf(stderr, "Não criou a fila de eventos!\n");
        exit(0);
    }
}

void ajusta_barco() {
    if (angle > 0) 
    {
        angle -= ALLEGRO_PI/18;
        
        if (angle < 0)
            angle = 0;
    }

    if (angle < 0)
    {
        angle += ALLEGRO_PI/18;

        if (angle > 0)
            angle = 0;
    }    
}

void inicializaAllegro()
{
    if (!al_init())
    {
        fprintf(stderr, "Falha ao inicializar a Allegro 5\n");
        return ;
    }

    if (!al_init_image_addon())
    {
        fprintf(stderr, "Falha ao inicializar a add-on_image Allegro 5\n");
        return ;
    }

    if (!al_init_primitives_addon())
    {
        fprintf(stderr, "Falha ao inicializar add-on de primitivas.\n");
        return ;
    }

    if (!al_install_keyboard())
    {
        fprintf(stderr, "Falha ao inicializar teclado\n");
        return ;
    }

    al_init_font_addon();

    if (!al_init_ttf_addon())
    {
        fprintf(stderr, "Allegro_ttf já inicializada!\n");
        return ;
    }
    
    fnt = al_load_font("images/gunplay.ttf", 40, 0);
    //fnt = al_load_font("images/atari1.ttf", 40, 0);
    if (!fnt)
    {
        fprintf(stderr, "Falha ao carregar fonte.\n");
        exit(0);
    }

    fnt_score = al_load_font("images/digiface.ttf", 20, 0);
    al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
}

void finalizaJogo() {
    DesalocaAmbiente(head);
    al_destroy_event_queue(fila);
    al_destroy_bitmap(barco);
    al_destroy_display(screen);

}

void print_pause_frame() {
    //al_draw_filled_rectangle(DISPLAY_WEIGHT/2.0, DISPLAY_HIGHT/2.5, DISPLAY_WEIGHT/2.0 + 100, DISPLAY_HIGHT/2.5 + 100, al_map_rgb(255,255,255));
    al_draw_text(fnt, al_map_rgb(0, 0, 0), DISPLAY_WEIGHT/2, DISPLAY_HIGHT/2.5, ALLEGRO_ALIGN_CENTRE, "P A U S E");
    al_flip_display();
}

void limpa_buffer_teclado (int nit) {
    int i;
    for (i = 0; i < nit; i++) {
        __fpurge(stdin);
    }
}
