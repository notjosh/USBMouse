//
//  USBMouse.swift
//  USBMouse
//
//  Created by joshua may on 15/6/16.
//  Copyright © 2016 joshua may. All rights reserved.
//

import Cocoa

enum USBMouseSpeed: UInt32 {
    case Off    = 0x0
    case Low    = 0x10
    case Medium = 0x20
    case High   = 0x40
}

enum USBMouseCharacter: UInt32, CustomStringConvertible {
    case Numeral0 = 0x0
    case Numeral1 = 0x1
    case Numeral2 = 0x2
    case Numeral3 = 0x3
    case Numeral4 = 0x4
    case Numeral5 = 0x5
    case Numeral6 = 0x6
    case Numeral7 = 0x7
    case Numeral8 = 0x8
    case Numeral9 = 0x9
    case Space = 0xA
    case F = 0xB
    case L = 0xC
    case B = 0xD  // note: lower case "b"
    case A = 0xE
    case E = 0xF

    static let allValues = [Numeral0,
                            Numeral1,
                            Numeral2,
                            Numeral3,
                            Numeral4,
                            Numeral5,
                            Numeral6,
                            Numeral7,
                            Numeral8,
                            Numeral9,
                            Space,
                            F,
                            L,
                            B,
                            A,
                            E]

    var description : String {
        switch self {
        case Numeral0: return "0"
        case Numeral1: return "1"
        case Numeral2: return "2"
        case Numeral3: return "3"
        case Numeral4: return "4"
        case Numeral5: return "5"
        case Numeral6: return "6"
        case Numeral7: return "7"
        case Numeral8: return "8"
        case Numeral9: return "9"
        case Space: return "[space]"
        case F: return "F"
        case L: return "L"
        case B: return "b"
        case A: return "A"
        case E: return "E"
        }
    }
}

protocol USBMouseDelegate {
    func notifyUp()
    func notifyDown()
}

class USBMouseController {
    private let MinimumSpeed = USBMouseSpeed.Low.rawValue;
    private let MaximumSpeed = USBMouseSpeed.High.rawValue;

    let delegate: USBMouseDelegate

    init(delegate: USBMouseDelegate) {
        self.delegate = delegate

        let p: UnsafeMutablePointer<Void> = UnsafeMutablePointer(
            OpaquePointer(
                bitPattern : Unmanaged.passUnretained(self)
            )
        )

        USBMouseIOInit({
                let _self = Unmanaged<USBMouseController>.fromOpaque(OpaquePointer($0!)).takeUnretainedValue()
                _self.delegate.notifyUp()
                USBMouseIOHandshake()
            }, p)
    }

    // MARK: Public API
    func stopMotor() {
        setMotorSpeed(speed: .Off)
    }

    func setMotorSpeed(speed: Double) {
        guard (speed >= 0.0) else { return; }
        guard (speed <= 1.0) else { return; }

        let intSpeed = UInt32(speed * Double(MaximumSpeed - MinimumSpeed)) + MinimumSpeed;
        setMotorSpeed(speed: intSpeed);
    }

    func setMotorSpeed(speed: USBMouseSpeed) {
        setMotorSpeed(speed: speed.rawValue);
    }

    func setLCD(c1: USBMouseCharacter, c2: USBMouseCharacter, c3: USBMouseCharacter, c4: USBMouseCharacter, c5: USBMouseCharacter) {
        setLCD(c1: c1.rawValue,
               c2: c2.rawValue,
               c3: c3.rawValue,
               c4: c4.rawValue,
               c5: c5.rawValue);
    }

    func setLCDValue(value: UInt32) {
        guard (value <= 99999) else { return; }

        return setLCD(c1: value / 10000 % 10,
                      c2: value / 1000  % 10,
                      c3: value / 100   % 10,
                      c4: value / 10    % 10,
                      c5: value         % 10);
    }

    func LCDIncrement() {
        poke(command: 0x66);
    }

    // MARK: Local API
    private func setMotorSpeed(speed: UInt32) {
        guard (speed >= MinimumSpeed || speed == 0) else { return; }
        guard (speed <= MaximumSpeed) else { return; }

        // add motor command bit
        let command = speed | 0x80;

        poke(command: command);
    }

    private func setLCD(c1: UInt32, c2: UInt32, c3: UInt32, c4: UInt32, c5: UInt32) {
        poke(command: 0x0A);
        poke(command: c1);
        poke(command: c2);
        poke(command: c3);
        poke(command: c4);
        poke(command: c5);
        poke(command: 0x39);

        // commit & force refresh
        poke(command: 0x05);
    }

    internal func poke(command: UInt32) {
        USBMouseIOPoke(command);
    }
}
