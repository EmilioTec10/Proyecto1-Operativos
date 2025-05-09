import pygame
import sys
import socket

# Constants
WIDTH, HEIGHT = 1000, 600
CAR_WIDTH, CAR_HEIGHT = 100, 60
LEFT_START_X = 10
RIGHT_START_X = WIDTH - CAR_WIDTH - 10
LEFT_START_Y = 150
RIGHT_START_Y = 150
CAR_SPACING = 130
LINES_PARKING = 2
MOVEMENT = 'none'

ASPHALT_COLOR = (50, 50, 50)
WHITE = (255, 255, 255)
YELLOW = (255, 255, 0)

# Starting conection
client = socket.socket()
client.connect(("localhost", 8080))


def parse_data(data_str):
    carros = []
    for carro in data_str.strip().split(';'):
        if carro:
            x, y, speed, movement, direction = map(int, carro.split(','))
            carros.append((x, y, movement, speed, direction))
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
    pygame.draw.line(surface, WHITE, (300, 200), (700, 200), 4)
    pygame.draw.line(surface, WHITE, (300, 400), (700, 400), 4)

    # Center dashed yellow line
    dash_length = 30
    gap = 20
    x = 300
    while x < 700:
        pygame.draw.line(surface, YELLOW, (x, 300), (x + dash_length, 300), 4)
        x += dash_length + gap

def draw_parking_lines(surface, car_positions):
    for pos in car_positions:
        x, y = pos
        pygame.draw.line(surface, WHITE, (x, y + CAR_HEIGHT + 30), (x + CAR_WIDTH, y + CAR_HEIGHT + 30), 2)
def draw_sign(direction):
    label = font.render(f"Direction: {direction.capitalize()}", True, (255, 255, 255))
    pygame.draw.rect(screen, (0, 0, 0), (WIDTH//2 - 120, 30, 240, 50))
    screen.blit(label, (WIDTH//2 - label.get_width()//2, 40))


def get_car(direction, priority):
    if priority == 0:
        image = load_car_image('images/car4.png', direction)
    elif priority == 1:
        image = load_car_image('images/car1.png', direction)
    else:
        image = load_car_image('images/car5.png', direction)
    return image

def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Scheduling Cars")

    clock = pygame.time.Clock()

    lines = []

    for i in range(LINES_PARKING):
        y_left = LEFT_START_Y + 130*i
        lines.append((LEFT_START_X, y_left))
        y_right = RIGHT_START_Y + 130*i
        lines.append((RIGHT_START_X, y_right))
        print(RIGHT_START_X)
    running = True
    while running:
        clock.tick(60)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        screen.fill(ASPHALT_COLOR)

        draw_lane_markings(screen)
        data = client.recv(1024).decode()
        carros = parse_data(data)

        # Draw lines
        draw_parking_lines(screen, lines)

        # Draw right cars and lines
        i = 0
        for x, y, movement, speed, direction in carros:
            if i == 3:
                i = 0
            img_carro = get_car(direction, i)
            i+=1
            screen.blit(img_carro, (x+speed*movement, y))

        pygame.display.flip()

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
