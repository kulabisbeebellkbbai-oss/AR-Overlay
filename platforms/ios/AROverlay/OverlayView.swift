import SwiftUI

struct OverlayElement: Identifiable {
    let id: String
    let type: String
    let x: CGFloat
    let y: CGFloat
    let width: CGFloat
    let height: CGFloat
    let z: Double
    let text: String
}

struct OverlayScene {
    let id: String
    let width: CGFloat
    let height: CGFloat
    let elements: [OverlayElement]
}

struct OverlayView: View {
    let scene: OverlayScene

    var body: some View {
        GeometryReader { geometry in
            let scaleX = geometry.size.width / scene.width
            let scaleY = geometry.size.height / scene.height
            ZStack(alignment: .topLeading) {
                ForEach(scene.elements.sorted { $0.z < $1.z }) { element in
                    if element.type == "rect" {
                        Rectangle()
                            .fill(Color(red: 0.06, green: 0.09, blue: 0.13).opacity(0.82))
                            .frame(width: element.width * scaleX, height: element.height * scaleY)
                            .position(x: (element.x + element.width / 2) * scaleX,
                                      y: (element.y + element.height / 2) * scaleY)
                    } else {
                        Text(element.text)
                            .foregroundStyle(element.id == "status" ? .yellow : .white)
                            .font(.system(size: element.id == "headline" ? 44 * scaleY : 26 * scaleY))
                            .frame(width: element.width * scaleX, height: element.height * scaleY)
                            .position(x: (element.x + element.width / 2) * scaleX,
                                      y: (element.y + element.height / 2) * scaleY)
                    }
                }
            }
        }
    }
}
