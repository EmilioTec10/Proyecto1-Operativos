import pygame
import sys

# Constants
WIDTH, HEIGHT = 800, 600
CAR_WIDTH, CAR_HEIGHT = 120, 80
LEFT_START_X = 10
RIGHT_START_X = WIDTH - CAR_WIDTH
CAR_SPACING = 120  # spacing between cars
CARS_PER_SIDE = 5

WHITE = (255, 255, 255)
ROAD_COLOR = (50, 50, 50)

def load_car_image(path, flip=False):
    """Loads and optionally flips a car image."""
    image = pygame.image.load(path)
    image = pygame.transform.scale(image, (CAR_WIDTH, CAR_HEIGHT))
    if flip:
        image = pygame.transform.flip(image, True, False)
    return image


def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Car Queues")

    clock = pygame.time.Clock()

    # Load car images
    car_right_image = load_car_image("images/car1_right.png")
    car_left_image = load_car_image("images/car1_right.png", flip=True)

    # Vertical alignment: fit cars above and below the road
    road_top_y = HEIGHT - 30
    road_bottom_y = HEIGHT // 2 + 40

    # Generate car positions
    left_cars = []
    right_cars = []

    for i in range(CARS_PER_SIDE):
        # Stack upwards from just above the road
        x_left = LEFT_START_X
        y_left = road_top_y - CAR_HEIGHT - (CARS_PER_SIDE - 1 - i) * CAR_SPACING
        left_cars.append((x_left, y_left))

        # Stack downwards from just below the road
        x_right = RIGHT_START_X
        y_right = road_top_y - CAR_HEIGHT - (CARS_PER_SIDE - 1 - i) * CAR_SPACING
        right_cars.append((x_right, y_right))

    running = True
    while running:
        clock.tick(60)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        # Draw background and road
        screen.fill(ROAD_COLOR)

        # Draw left cars
        for pos in left_cars:
            screen.blit(car_left_image, pos)

        # Draw right cars
        for pos in right_cars:
            screen.blit(car_right_image, pos)

        pygame.display.flip()

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
