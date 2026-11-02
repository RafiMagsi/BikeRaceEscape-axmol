import cv2
import numpy as np

def generate_xml_collision_string(image_path, target_max_vertices=5, center_relative=True):
    """
    Traces image alpha channel contours, creates an optimized convex hull,
    and returns perfectly formatted Axmol XML string blocks.
    """
    img = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)
    if img is None or img.shape[2] < 4:
        return "<!-- Error: Image missing alpha channel for tracing -->"
        
    alpha = img[:, :, 3]
    _, thresh = cv2.threshold(alpha, 15, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    if not contours:
        return "<array/>"
        
    main_contour = max(contours, key=cv2.contourArea)
    
    # Iterative Douglas-Peucker reduction to hit target vertex count limits
    epsilon = 0.005 * cv2.arcLength(main_contour, True)
    approx = cv2.approxPolyDP(main_contour, epsilon, True)
    hull = cv2.convexHull(approx, clockwise=False) # Guarantees strict Counter-Clockwise (CCW) winding
    
    while len(hull) > target_max_vertices:
        epsilon += 0.005 * cv2.arcLength(main_contour, True)
        approx = cv2.approxPolyDP(main_contour, epsilon, True)
        hull = cv2.convexHull(approx, clockwise=False)

    h, w = alpha.shape
    cx, cy = w / 2.0, h / 2.0
    
    xml_lines = ["<array>"]
    for point in hull:
        x, y = point[0]
        if center_relative:
            # Map screen-space pixel coordinates into center-relative coordinate space
            rel_x = round(float(x - cx), 2)
            rel_y = round(float(cy - y), 2)
        else:
            rel_x, rel_y = round(float(x), 2), round(float(y), 2)
            
        xml_lines.append(f"\t\t\t\t<string>{{{rel_x}, {rel_y}}}</string>")
    xml_lines.append("\t\t\t</array>")
    
    return "\n".join(xml_lines)
