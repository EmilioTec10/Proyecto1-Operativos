import pygame
import json
import GUI

pygame.init()
screen = pygame.display.set_mode((1600, 700))
pygame.display.set_caption("Menu")
clock = pygame.time.Clock()

font = pygame.font.SysFont("Arial", 20)
WHITE, BLACK, RED, YELLOW, GREY = (255,255,255), (0,0,0), (200,0,0), (255, 255, 0), (50, 50, 50)
RIGHT = 0
LEFT = 0
cars_added = []

input_boxes = {
    "W": {"rect": pygame.Rect(410, 130, 100, 30), "text": "", "active": False},
    "Letrero": {"rect": pygame.Rect(820, 130, 100, 30), "text": "", "active": False},
    "LargoCalle": {"rect": pygame.Rect(830, 325, 100, 30), "text": "", "active": False},
    "Velocidad": {"rect": pygame.Rect(840, 360, 100, 30), "text": "", "active": False}
}

flow_options = [
    {"label": "Equidad", "pos": (400, 80), "selected": True},
    {"label": "Letrero", "pos": (760, 80), "selected": False},
    {"label": "FIFO",    "pos": (1150, 80), "selected": False}
]

scheduler_options = [
    {"label": "RR",           "pos": (340, 270), "selected": True},
    {"label": "Prioridad",    "pos": (560, 270), "selected": False},
    {"label": "SJF",          "pos": (780, 270), "selected": False},
    {"label": "FCFS",         "pos": (1000, 270), "selected": False},
    {"label": "Tiempo Real",  "pos": (1220, 270), "selected": False}
]
car_options = [
    {"label": "Normal", "pos": (400, 80), "selected": True},
    {"label": "Deportivo", "pos": (760, 80), "selected": False},
    {"label": "Emergencia",    "pos": (1150, 80), "selected": False}
]

def draw_text(text, pos, color=WHITE):
    surface = font.render(text, True, color)
    screen.blit(surface, pos)

def draw_input_boxes():
    for key, box in input_boxes.items():
        color = RED if box["active"] else BLACK
        pygame.draw.rect(screen, color, box["rect"], 2)
        txt_surface = font.render(box["text"], True, WHITE)
        screen.blit(txt_surface, (box["rect"].x+5, box["rect"].y+5))

def draw_radio_buttons():
    for option in flow_options:
        color = YELLOW if option["selected"] else BLACK
        pygame.draw.circle(screen, color, option["pos"], 10)
        draw_text(option["label"], (option["pos"][0] + 20, option["pos"][1] - 10))

def draw_scheduler_options():
    draw_text("Seleccione el tipo de Calendarización:", (640, 210), WHITE)
    for option in scheduler_options:
        color = YELLOW if option["selected"] else BLACK
        pygame.draw.circle(screen, color, option["pos"], 10)
        draw_text(option["label"], (option["pos"][0] + 20, option["pos"][1] - 10))

def handle_radio_click(pos):
    for option in flow_options:
        if pygame.Rect(option["pos"][0]-10, option["pos"][1]-10, 20, 20).collidepoint(pos):
            for o in flow_options: o["selected"] = False
            option["selected"] = True
            return
    for option in scheduler_options:
        if pygame.Rect(option["pos"][0]-10, option["pos"][1]-10, 20, 20).collidepoint(pos):
            for o in scheduler_options: o["selected"] = False
            option["selected"] = True
            return

def validate_counts():
    try:
        l_total = sum(int(input_boxes[k]["text"]) if input_boxes[k]["text"].isdigit() else 0 for k in ["L-Normal", "L-Deportivo", "L-Emergencia"])
        r_total = sum(int(input_boxes[k]["text"]) if input_boxes[k]["text"].isdigit() else 0 for k in ["R-Normal", "R-Deportivo", "R-Emergencia"])
        return l_total <= 5 and r_total <= 5, l_total, r_total
    except:
        return False, 0, 0

def save_config(config):
    with open("configuracion.json", "w") as f:
        json.dump(config, f, indent=4)

error_message = ""
running = True

while running:
    screen.fill(GREY)
    draw_text("Seleccione Método de control de flujo:", (640, 20))
    draw_text("W:", (380, 135))
    draw_text("Tiempo letrero:", (680, 135))
    draw_text("q. Normal en Izquierda",     (50, 495))
    draw_text("w. Deportivo en Izquierda",  (300, 495))
    draw_text("e. Emergencia en Izquierda", (550, 495))
    draw_text("a. Normal en Derecha",     (830, 495))
    draw_text("s. Deportivo en Derecha",  (1050, 495))
    draw_text("d. Emergencia en Derecha", (1300, 495))
    draw_text("Presiona la tecla del tipo de carro que desea agregar:", (570, 440))
    draw_text("Ingrese Largo de calle:", (620, 330))
    draw_text("Ingrese la Velocidad Base:", (600, 365))


    draw_input_boxes()
    draw_radio_buttons()
    draw_scheduler_options()
    draw_text("Presiona Enter para continuar...", (670, 655), WHITE)
    if error_message:
        draw_text(error_message, (620, 605), RED)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.MOUSEBUTTONDOWN:
            pos = event.pos
            for box in input_boxes.values():
                box["active"] = box["rect"].collidepoint(pos)
            handle_radio_click(pos)

        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_RETURN:
                #valid, l_sum, r_sum = validate_counts()
                flujo = next(opt["label"] for opt in flow_options if opt["selected"])
                scheduler = next(opt["label"] for opt in scheduler_options if opt["selected"])

                # Validaciones adicionales
                if flujo == "Equidad":
                    if not input_boxes["W"]["text"].isdigit() or int(input_boxes["W"]["text"]) > 9:
                        error_message = "W debe ser un número entero menor a 10"
                        continue
                if flujo == "Letrero":
                    if not input_boxes["Letrero"]["text"].isdigit() or int(input_boxes["Letrero"]["text"]) > 29:
                        error_message = "Tiempo de letrero debe ser < 30"
                        continue
                #if not valid:
                    #error_message = f"Máx 5 carros por lado. Izq: {l_sum}, Der: {r_sum}"
                    #continue
                if not input_boxes["LargoCalle"]["text"].isdigit() or not (
                        100 <= int(input_boxes["LargoCalle"]["text"]) <= 500):
                    error_message = "Largo de la calle debe ser un número entre 100 y 500"
                    continue
                if not input_boxes["Velocidad"]["text"].isdigit() or not (
                        1 <= int(input_boxes["Velocidad"]["text"]) <= 30):
                    error_message = "La velocidad base debe ser un número entre 1 y 30"
                    continue
                error_message = ""
                base = int(input_boxes["Velocidad"]["text"])
                config = {
                    "flujo": flujo,
                    "scheduler": scheduler,
                    "W": input_boxes["W"]["text"],
                    "tiempo_letrero": input_boxes["Letrero"]["text"],
                    "largo_calle": input_boxes["LargoCalle"]["text"],
                    "velocidad": {
                        "normal": str(base),
                        "deportivo": str(base*2),
                        "emergencia": str(base*3)
                    },
                    "cars": cars_added
                }
                save_config(config)
                print("Configuración guardada exitosamente.")
                running = False
                GUI.main()
            elif event.key == pygame.K_q:
                if LEFT == 5:
                    error_message = "Solo se pueden agregar 5 carros por lado"
                else:
                    cars_added.append("normal_izquierda")
                    error_message = "Se agrego un carro normal en la izquierda"
                    LEFT += 1
            elif event.key == pygame.K_w:
                if LEFT == 5:
                    error_message = "Solo se pueden agregar 5 carros por lado"
                else:
                    cars_added.append("deportivo_izquierda")
                    error_message = "Se agrego un carro deportivo en la izquierda"
                    LEFT += 1
            elif event.key == pygame.K_e:
                if LEFT == 5:
                    error_message = "Solo se pueden agregar 5 carros por lado"
                else:
                    cars_added.append("emergencia_izquierda")
                    error_message = "Se agrego un carro de emergencia en la izquierda"
                    LEFT += 1
            elif event.key == pygame.K_a:
                if LEFT == 5:
                    error_message = "Solo se pueden agregar 5 carros por lado"
                else:
                    cars_added.append("normal_derecha")
                    error_message = "Se agrego un carro normal en la derecha"
                    RIGHT += 1
            elif event.key == pygame.K_s:
                if LEFT == 5:
                    error_message = "Solo se pueden agregar 5 carros por lado"
                else:
                    cars_added.append("deportivo_derecha")
                    error_message = "Se agrego un carro deportivo en la derecha"
                    RIGHT += 1
            elif event.key == pygame.K_d:
                if LEFT == 5:
                    error_message = "Solo se pueden agregar 5 carros por lado"
                else:
                    cars_added.append("emergencia_derecha")
                    error_message = "Se agrego un carro de emergencia en la derecha"
                    RIGHT += 1
            else:
                for box in input_boxes.values():
                    if box["active"]:
                        if event.key == pygame.K_BACKSPACE:
                            box["text"] = box["text"][:-1]
                        elif event.unicode.isdigit():
                            box["text"] += event.unicode

    pygame.display.flip()
    clock.tick(30)

pygame.quit()
