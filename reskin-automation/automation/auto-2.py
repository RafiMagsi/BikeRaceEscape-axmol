import cv2
import numpy as np

def compile_dynamic_axmol_sprite(image_path, sprite_key, target_max_vertices=8):
    """
    Analyzes a newly generated reskin sprite frame of arbitrary shape/size.
    Calculates dynamic source sizes, transparent offsets, and outputs clean 
    Box2D/Axmol XML plist configurations for both the sheet layout and physics nodes.
    """
    # Load image with alpha layer channel
    img = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)
    if img is None or img.shape[2] < 4:
        return None
        
    alpha = img[:, :, 3]
    h, w = alpha.shape
    
    # 1. Calculate Tight Visual Bounding Rect (sourceColorRect)
    pts = np.argwhere(thresh_mask := cv2.threshold(alpha, 10, 255, cv2.THRESH_BINARY)[1])
    if pts.size == 0:
        return None
        
    y1, x1 = pts.min(axis=0)
    y2, x2 = pts.max(axis=0)
    visible_w = int((x2 - x1) + 1)
    visible_h = int((y2 - y1) + 1)
    
    # Calculate offset relative to original center space
    offset_x = round(float((x1 + x2) / 2.0 - w / 2.0), 1)
    offset_y = round(float(h / 2.0 - (y1 + y2) / 2.0), 1)
    
    # 2. Extract Convex Hull Physics Vertices (Strict CCW Order)
    contours, _ = cv2.findContours(thresh_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    main_contour = max(contours, key=cv2.contourArea)
    
    epsilon = 0.005 * cv2.arcLength(main_contour, True)
    approx = cv2.approxPolyDP(main_contour, epsilon, True)
    hull = cv2.convexHull(approx, clockwise=False)
    
    while len(hull) > target_max_vertices:
        epsilon += 0.005 * cv2.arcLength(main_contour, True)
        approx = cv2.approxPolyDP(main_contour, epsilon, True)
        hull = cv2.convexHull(approx, clockwise=False)
        
    # Format structural physics block lines
    physics_xml = ["\t\t\t<key>collision</key>\n\t\t\t<array>"]
    cx, cy = w / 2.0, h / 2.0
    for pt in hull:
        x, y = pt[0]
        rel_x = round(float(x - cx), 2)
        rel_y = round(float(cy - y), 2)
        physics_xml.append(f"\t\t\t\t<string>{{{rel_x}, {rel_y}}}</string>")
    physics_xml.append("\t\t\t</array>")
    
    # 3. Format Structural Sprite Sheet Metadata Node Block
    sheet_xml = f"""
		<key>{sprite_key}</key>
		<dict>
			<key>frame</key>
			<string>{{{{NEW_PACK_X, NEW_PACK_Y}}, {{{visible_w}, {visible_h}}}}}</string>
			<key>offset</key>
			<string>{{{offset_x}, {offset_y}}}</string>
			<key>rotated</key>
			<false/>
			<key>sourceColorRect</key>
			<string>{{{{{x1}, {y1}}}, {{{visible_w}, {visible_h}}}}}</string>
			<key>sourceSize</key>
			<string>{{{w}, {h}}}</string>
		</dict>"""
        
    return {
        "sheet_node": sheet_xml,
        "physics_node": "\n".join(physics_xml),
        "dimensions": (w, h)
    }
