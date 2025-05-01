import pygame
import sys

# Constants
WIDTH, HEIGHT = 1000, 600
CAR_WIDTH, CAR_HEIGHT = 100, 60
LEFT_START_X = 10
RIGHT_START_X = WIDTH - CAR_WIDTH - 10
LEFT_START_Y = 20
RIGHT_START_Y = 20
CAR_SPACING = 130
CARS_PER_SIDE = 5
MOVEMENT = 'none'

ASPHALT_COLOR = (50, 50, 50)
WHITE = (255, 255, 255)
YELLOW = (255, 255, 0)


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
def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Scheduling Cars")

    clock = pygame.time.Clock()
    car_right_image = []
    car_left_image = []
    car_right_image.append(load_car_image("images/car1.png"))
    car_left_image.append(load_car_image("images/car1.png", flip=True))
    car_right_image.append(load_car_image("images/car2.png"))
    car_left_image.append(load_car_image("images/car2.png", flip=True))
    car_right_image.append(load_car_image("images/car3.png"))
    car_left_image.append(load_car_image("images/car3.png", flip=True))
    car_right_image.append(load_car_image("images/car4.png"))
    car_left_image.append(load_car_image("images/car4.png", flip=True))
    car_right_image.append(load_car_image("images/car5.png"))
    car_left_image.append(load_car_image("images/car5.png", flip=True))

    left_cars = []
    right_cars = []

    for i in range(CARS_PER_SIDE):
        x_left = LEFT_START_X
        y_left = LEFT_START_Y + 130*i
        left_cars.append((x_left, y_left))

        x_right = RIGHT_START_X
        y_right = RIGHT_START_Y + 130*i
        right_cars.append((x_right, y_right))

    running = True
    while running:
        clock.tick(60)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        screen.fill(ASPHALT_COLOR)

        draw_lane_markings(screen)



        # Draw left cars and lines
        i = 0
        for pos in left_cars:
            if MOVEMENT == "left":
                pass
            screen.blit(car_left_image[i], pos)
            i+=1

        draw_parking_lines(screen, left_cars)

        # Draw right cars and lines
        i = 0
        for pos in right_cars:
            if MOVEMENT == "right":
                pass
            screen.blit(car_right_image[i], pos)
            i+=1
        draw_parking_lines(screen, right_cars)

        pygame.display.flip()

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
