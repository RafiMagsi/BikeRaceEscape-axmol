// Native CoreText/AppKit lettering; no third-party font rasterizer dependency.
// Usage: binary text size hexColor bold|regular output.png
import AppKit
let args = CommandLine.arguments
guard args.count == 6, let size = Double(args[2]) else { fatalError("text size hex weight output") }
let color = args[3].replacingOccurrences(of: "#", with: "")
let rgb = UInt32(color, radix:16) ?? 0xffffff
let font = NSFont(name: args[4] == "bold" ? "Arial-BoldMT" : "ArialMT", size:size) ?? NSFont.systemFont(ofSize:size)
let text = NSAttributedString(string:args[1],attributes:[.font:font,.foregroundColor:NSColor(srgbRed:CGFloat((rgb>>16)&255)/255,green:CGFloat((rgb>>8)&255)/255,blue:CGFloat(rgb&255)/255,alpha:1)])
let bounds = text.size()
let w = Int(ceil(bounds.width))+8, h=Int(ceil(bounds.height))+8
let bitmap=NSBitmapImageRep(bitmapDataPlanes:nil,pixelsWide:w,pixelsHigh:h,bitsPerSample:8,samplesPerPixel:4,hasAlpha:true,isPlanar:false,colorSpaceName:.deviceRGB,bytesPerRow:0,bitsPerPixel:0)!
NSGraphicsContext.saveGraphicsState()
NSGraphicsContext.current=NSGraphicsContext(bitmapImageRep:bitmap)
text.draw(at:NSPoint(x:4,y:4))
NSGraphicsContext.restoreGraphicsState()
try bitmap.representation(using:.png,properties:[:])!.write(to:URL(fileURLWithPath:args[5]))
