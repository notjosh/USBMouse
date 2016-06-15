//
//  ViewController.swift
//  USBMouse
//
//  Created by joshua may on 15/6/16.
//  Copyright © 2016 joshua may. All rights reserved.
//

import Cocoa

class ViewController: NSViewController, USBMouseDelegate {
    var controller: USBMouseController! = nil

    @IBOutlet var statusLabel: NSTextField!;

    @IBOutlet var pedalSpeedSlider: NSSlider!;

    @IBOutlet var popUpButton1: NSPopUpButton!;
    @IBOutlet var popUpButton2: NSPopUpButton!;
    @IBOutlet var popUpButton3: NSPopUpButton!;
    @IBOutlet var popUpButton4: NSPopUpButton!;
    @IBOutlet var popUpButton5: NSPopUpButton!;

    override func viewDidLoad() {
        super.viewDidLoad()

        statusLabel.stringValue = "Disconnected"

        controller = USBMouseController(delegate: self)

        [popUpButton1, popUpButton2, popUpButton3, popUpButton4, popUpButton5].forEach { (b) in
            b?.removeAllItems();
            b?.addItems(withTitles: USBMouseCharacter.allValues.map({ (v) -> String in
                return v.description
            }));
        }

        pedalSpeedSlider.doubleValue = 0.0

        popUpButton1.selectItem(at: USBMouseCharacter.allValues.index(of: .F)!)
        popUpButton2.selectItem(at: USBMouseCharacter.allValues.index(of: .E)!)
        popUpButton3.selectItem(at: USBMouseCharacter.allValues.index(of: .E)!)
        popUpButton4.selectItem(at: USBMouseCharacter.allValues.index(of: .L)!)
        popUpButton5.selectItem(at: USBMouseCharacter.allValues.index(of: .Numeral5)!)
    }

    @IBAction func updateLCD(sender: AnyObject) {
        controller.setLCD(c1: USBMouseCharacter.allValues[popUpButton1.indexOfSelectedItem],
                                  c2: USBMouseCharacter.allValues[popUpButton2.indexOfSelectedItem],
                                  c3: USBMouseCharacter.allValues[popUpButton3.indexOfSelectedItem],
                                  c4: USBMouseCharacter.allValues[popUpButton4.indexOfSelectedItem],
                                  c5: USBMouseCharacter.allValues[popUpButton5.indexOfSelectedItem])
    }

    @IBAction func updateMouseSpeed(sender: AnyObject) {
        let pedalSpeed = pedalSpeedSlider.doubleValue;

        if (pedalSpeed < 0.01) {
            controller.stopMotor()
        } else {
            controller.setMotorSpeed(speed: pedalSpeed)
        }
    }

    // MARK: USBMouseDelegate
    func notifyUp() {
        statusLabel.stringValue = "Connected"
    }

    func notifyDown() {
        statusLabel.stringValue = "Disonnected"
    }
}

