import re

def automate_text_reskin(plist_content, conversion_map):
    """
    Directly targets textual nodes, strings, descriptions, and structural names
    without breaking XML structure.
    """
    updated_content = plist_content
    for original_string, replacement_string in conversion_map.items():
        # Match standard <string> original </string> tags
        pattern = re.compile(rf"<string>{re.escape(original_string)}</string>")
        updated_content = pattern.sub(f"<string>{replacement_string}</string>", updated_content)
    return updated_content

# Example usage mapping old titles to the new Sci-Fi Synthwave theme
theme_map = {
    "Motorcycle Bike Race Road Escape : Speed Racing Raod Game": "Neon Velocity: Cyber Glide 2026",
    "Red Motorcycle Bike": "Cyber Blade - Prototype X",
    "Blue Dirt Bike": "Volt Courser - Kinetic Edition",
    "Silver Street Bike": "Apex Mirage - Stealth Glider",
    "Zombie Allegator": "Plasma Barrier Hazard",
    "Zombie Lizard": "Laser Spike Trap",
    "Zombie Sewer Rat": "Proximity Plasma Drone",
    "Gold Wheels": "Neon Energy Cores",
    "spinning rims": "Quantum Singularity Discs"
}
