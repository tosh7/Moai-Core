import Foudantion
import UIKit

final class Elevetor {
    enum Direcion {
        case up
        case down
    }

    private let maxFloor: Int
    private let minFloor: Int
    var currentFloor: Int = 1
    var requestedFloor: [Int] = []
    var direction: Direcion?

    init(maxFloor: Int, minFloor: Int) {
        self.maxFloor = maxFloor
        self.minFloor = minFloor
    }

    func request(from floor: Int, max: Int) {
    }
}