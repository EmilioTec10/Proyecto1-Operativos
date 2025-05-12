import pygame
import json
import GUI

pygame.init()
screen = pygame.display.set_mode((1000, 600))
pygame.display.set_caption("Menu")
clock = pygame.time.Clock()

font = pygame.font.SysFont("Arial", 20)
WHITE, BLACK, RED, YELLOW, GREY = (255,255,255), (0,0,0), (200,0,0), (255, 255, 0), (50, 50, 50)

input_boxes = {
    "W": {"rect": pygame.Rect(290, 50, 100, 30), "text": "", "active": False},
    "Letrero": {"rect": pygame.Rect(290, 100, 100, 30), "text": "", "active": False},
    "LargoCalle": {"rect": pygame.Rect(290, 465, 100, 30), "text": "", "active": False},
    "Velocidad": {"rect": pygame.Rect(290, 500, 100, 30), "text": "", "active": False},
    "L-Normal": {"rect": pygame.Rect(290, 190, 50, 30), "text": "", "active": False},
    "L-Deportivo": {"rect": pygame.Rect(290, 225, 50, 30), "text": "", "active": False},
    "L-Emergencia": {"rect": pygame.Rect(290, 260, 50, 30), "text": "", "active": False},
    "R-Normal": {"rect": pygame.Rect(290, 340, 50, 30), "text": "", "active": False},
    "R-Deportivo": {"rect": pygame.Rect(290, 375, 50, 30), "text": "", "active": False},
    "R-Emergencia": {"rect": pygame.Rect(290, 410, 50, 30), "text": "", "active": False},
}

flow_options = [
    {"label": "Equidad", "pos": (600, 50), "selected": True},
    {"label": "Letrero", "pos": (600, 100), "selected": False},
    {"label": "FIFO",    "pos": (600, 150), "selected": False}
]

scheduler_options = [
    {"label": "RR",           "pos": (600, 250), "selected": True},
    {"label": "Prioridad",    "pos": (600, 285), "selected": False},
    {"label": "SJF",          "pos": (600, 320), "selected": False},
    {"label": "FCFS",         "pos": (600, 355), "selected": False},
    {"label": "Tiempo Real",  "pos": (600, 390), "selected": False}
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
    draw_text("Calendarización:", (580, 210), WHITE)
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
    draw_text("Método de control de flujo:", (50, 20))
    draw_text("W:", (100, 55))
    draw_text("Tiempo letrero:", (50, 105))
    draw_text("Cantidad de Carros Izquierda (máx 5):", (50, 160))
    draw_text("Normal:",     (70, 195))
    draw_text("Deportivo:",  (70, 230))
    draw_text("Emergencia:", (70, 275))
    draw_text("Cantidad de Carros Derecha (máx 5):", (50, 310))
    draw_text("Normal:",     (70, 345))
    draw_text("Deportivo:",  (70, 380))
    draw_text("Emergencia:", (70, 415))
    draw_text("Ingrese Largo de calle:", (50, 470))
    draw_text("Ingrese la Velocidad Base:", (50, 505))


    draw_input_boxes()
    draw_radio_buttons()
    draw_scheduler_options()
    draw_text("Presiona Enter para continuar...", (350, 575), WHITE)
    if error_message:
        draw_text(error_message, (330, 540), RED)

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
                valid, l_sum, r_sum = validate_counts()
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
                if not valid:
                    error_message = f"Máx 5 carros por lado. Izq: {l_sum}, Der: {r_sum}"
                    continue
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
                    "izquierda": {
                        "normal": input_boxes["L-Normal"]["text"],
                        "deportivo": input_boxes["L-Deportivo"]["text"],
                        "emergencia": input_boxes["L-Emergencia"]["text"]
                    },
                    "derecha": {
                        "normal": input_boxes["R-Normal"]["text"],
                        "deportivo": input_boxes["R-Deportivo"]["text"],
                        "emergencia": input_boxes["R-Emergencia"]["text"]
                    }
                }
                save_config(config)
                print("Configuración guardada exitosamente.")
                running = False
                GUI.main()
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
