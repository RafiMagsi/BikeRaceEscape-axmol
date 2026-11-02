import cv2
import numpy as np

def extract_physics_vertices(image_path, alpha_threshold=10, max_vertices=8):
    """
    Reads a sprite with an alpha channel, traces its boundary, 
    simplifies the shape into a convex hull satisfying Axmol/Box2D constraints,
    and returns the vertices in Counter-Clockwise (CCW) order.
    """
    # Load image with alpha channel
    img = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)
    if img is None or img.shape[2] < 4:
        raise ValueError("Image must be a PNG with a valid Alpha channel.")
        
    # Extract alpha channel
    alpha_channel = img[:, :, 3]
    
    # Threshold alpha channel to create a binary mask
    _, binary_mask = cv2.threshold(alpha_channel, alpha_threshold, 255, cv2.THRESH_BINARY)
    
    # Find outer contours
    contours, _ = cv2.findContours(binary_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return []
        
    # Pick the largest contour found (assuming main sprite body)
    main_contour = max(contours, key=cv2.contourArea)
    
    # Get the image dimensions for origin translation if needed
    height, width = alpha_channel.shape
    center_x, center_y = width / 2.0, height / 2.0
    
    # Simplify the contour using Ramer-Douglas-Peucker (RDP) algorithm
    # Tweak epsilon dynamically to target max_vertices constraint
    epsilon = 0.01 * cv2.arcLength(main_contour, True)
    approx_contour = cv2.approxPolyDP(main_contour, epsilon, True)
    
    # Box2D engines require convex shapes. Create a convex hull from the simplified points.
    convex_hull = cv2.convexHull(approx_contour, clockwise=False) 
    
    # If the hull still exceeds max vertices allowed by Box2D (default 8), simplify further
    while len(convex_hull) > max_vertices:
        epsilon += 0.005 * cv2.arcLength(main_contour, True)
        approx_contour = cv2.approxPolyDP(main_contour, epsilon, True)
        convex_hull = cv2.convexHull(approx_contour, clockwise=False)
        
    # Format vertices relative to the center origin (standard for many physics tools)
    physics_vertices = []
    for point in convex_hull:
        x, y = point[0]
        # Translate to center-relative space and invert Y-axis if engine coordinates are bottom-left based
        rel_x = x - center_x
        rel_y = center_y - y  # Standard inversion for screen-space to physics-space mapping
        physics_vertices.append((round(rel_x, 2), round(rel_y, 2)))
        
    return physics_vertices

# Example trigger usage by the agent:
# coordinates = extract_physics_vertices("new_player_run.png")
# print("Generated Box2D Vertices (CCW):", coordinates)
