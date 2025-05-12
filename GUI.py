from tarfile import LENGTH_NAME

import pygame
import sys
import socket


# Constants
WIDTH, HEIGHT = 1600, 900
CAR_WIDTH, CAR_HEIGHT = 100, 60
LEFT_START_X = 10
RIGHT_START_X = WIDTH - CAR_WIDTH - 10
LEFT_START_Y = 50
RIGHT_START_Y = 50
CAR_SPACING = 120
LINES_PARKING = 4
MOVEMENT = 'none'
speed = 5
LEFT = 0
RIGHT = 0
LENGTH_STREET = 700

ASPHALT_COLOR = (50, 50, 50)
WHITE = (255, 255, 255)
YELLOW = (255, 255, 0)
RED = (255, 0, 0)

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Scheduling Cars")
clock = pygame.time.Clock()


# Crear una fuente
font = pygame.font.SysFont("Arial", 20)


def parse_data(data_str):
    carros = []
    for carro in data_str.strip().split(';'):
        if carro:
            orden, type, direction = map(int, carro.split(','))
            carros.append((orden, type, direction ))
    return carros
def load_car_image(path, flip=False):
    image = pygame.image.load(path)
    image = pygame.transform.scale(image, (CAR_WIDTH, CAR_HEIGHT))
    if flip:
        image = pygame.transform.flip(image, True, False)
    return image
def draw_sign(direction):
    label = font.render(f"Direction: {direction.capitalize()}", True, (255, 255, 255))
    pygame.draw.rect(screen, (0, 0, 0), (WIDTH//2 - 120, 30, 240, 50))
    screen.blit(label, (WIDTH//2 - label.get_width()//2, 40))
def draw_lane_markings(surface):
    # Border lines (white)
    pygame.draw.line(surface, WHITE, (800 - LENGTH_STREET//2, 240), (800 + LENGTH_STREET//2, 240), 4)
    pygame.draw.line(surface, WHITE, (800 - LENGTH_STREET//2, 380), (800 + LENGTH_STREET//2, 380), 4)

    # Center dashed yellow line
    dash_length = 30
    gap = 20
    x = 800 - LENGTH_STREET//2
    while x < 800 + LENGTH_STREET//2:
        pygame.draw.line(surface, YELLOW, (x, 310), (x + dash_length, 310), 4)
        x += dash_length + gap

def draw_parking_lines(surface, car_positions):
    for pos in car_positions:
        x, y = pos
        pygame.draw.line(surface, WHITE, (x, y + CAR_HEIGHT + 30), (x + CAR_WIDTH, y + CAR_HEIGHT + 30), 2)
def draw_sign(direction):
    label = font.render(f"Direction: {direction.capitalize()}", True, (255, 255, 255))
    pygame.draw.rect(screen, (0, 0, 0), (WIDTH//2 - 120, 30, 240, 50))
    screen.blit(label, (WIDTH//2 - label.get_width()//2, 40))

def draw_text(text, pos, color=WHITE):
    surface = font.render(text, True, color)
    screen.blit(surface, pos)

def get_car(direction, priority):
    if priority == 0:
        image = load_car_image('images/car4.png', direction)
    elif priority == 1:
        image = load_car_image('images/car1.png', direction)
    elif priority == 2:
        image = load_car_image('images/car5.png', direction)
    return image

def main():
    # Starting conection
    client = socket.socket()
    client.connect(("localhost", 8000))
    lines = []
    actual = 0
    x_actual_right = 800 + LENGTH_STREET // 2 + CAR_WIDTH
    x_actual_left = 800 - LENGTH_STREET // 2 - CAR_WIDTH
    y_actual_right = 240
    y_actual_left = 320
    x_right = RIGHT_START_X
    x_left = LEFT_START_X
    y_right = RIGHT_START_Y
    y_left = LEFT_START_Y
    x = 0
    y = 0
    data = client.recv(1024).decode()
    carros = parse_data(data)
    for i in range(LINES_PARKING):
        y_left = LEFT_START_Y + 120*i
        lines.append((LEFT_START_X, y_left))
        y_right = RIGHT_START_Y + 120*i
        lines.append((RIGHT_START_X, y_right))
    running = True
    while running:
        clock.tick(60)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_q:
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



        screen.fill(ASPHALT_COLOR)
        draw_lane_markings(screen)

        # Draw lines
        draw_parking_lines(screen, lines)
        draw_text("q. Normal en Izquierda", (50, 820))
        draw_text("w. Deportivo en Izquierda", (300, 820))
        draw_text("e. Emergencia en Izquierda", (550, 820))
        draw_text("a. Normal en Derecha", (830, 820))
        draw_text("s. Deportivo en Derecha", (1050, 820))
        draw_text("d. Emergencia en Derecha", (1300, 820))
        draw_text("Presiona la tecla del tipo de carro que desea agregar:", (570, 860))
        if error_message:
            draw_text(error_message, (620, 780), RED)
        new_cars = []
        # Draw cars
        for orden, type, direction  in carros:
            img_carro = get_car(direction, type)
            new_cars.append((orden, type, direction))
            if direction == 0:
                x = x_right
                y = y_right
                y_right += CAR_SPACING
                if actual == orden:
                    x = x_actual_right
                    y = y_actual_right
                    if x_actual_right <= 800 - LENGTH_STREET//2-CAR_WIDTH:
                        actual += 1
                        new_cars = []
                        x_actual_right = 800 + LENGTH_STREET // 2 + CAR_WIDTH
                        #Se tiene que enviar mensaje para que se elimine el hilo y se elimine de la lista en c
                    elif type == 0:
                        x_actual_right -= speed
                    elif type == 1:
                        x_actual_right -= 2*speed
                    elif type == 2:
                        x_actual_right -= 3*speed
            elif direction == 1:
                x = x_left
                y = y_left
                y_left += CAR_SPACING
                if actual == orden:
                    x = x_actual_left
                    y = y_actual_left
                    if x_actual_left >= 800 + LENGTH_STREET // 2 + CAR_WIDTH:
                        actual += 1
                        new_cars = []
                        x_actual_left = 800 - LENGTH_STREET // 2 - CAR_WIDTH
                        # Se tiene que enviar mensaje para que se elimine el hilo y se elimine de la lista en c
                    elif type == 0:
                        x_actual_left += speed
                    elif type == 1:
                        x_actual_left += 2*speed
                    elif type == 2:
                        x_actual_left += 3*speed

            screen.blit(img_carro, (x, y))
        x_right = RIGHT_START_X
        x_left = LEFT_START_X
        y_right = RIGHT_START_Y
        y_left = LEFT_START_Y
        carros = new_cars
        pygame.display.flip()


    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()

