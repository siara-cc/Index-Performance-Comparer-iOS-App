//
//  ViewController.swift
//  Index Research
//
//  Created by Arundale Ramanathan on 31/03/18.
//  Copyright © 2018 Siara Logics (cc). All rights reserved.
//

import UIKit

class ViewController: UIViewController, UIPickerViewDelegate, UIPickerViewDataSource {

    @IBOutlet weak var pvIdxSel2: UIPickerView!
    @IBOutlet weak var pvIdxSel3: UIPickerView!
    @IBOutlet weak var pvCharSet: UIPickerView!
    @IBOutlet weak var pvDataSet: UIPickerView!
    @IBOutlet weak var etIdxLen: UITextField!
    @IBOutlet weak var etCount: UITextField!
    @IBOutlet weak var etKeyLen: UITextField!
    @IBOutlet weak var etValueLen: UITextField!
    @IBOutlet weak var etARTPut: UITextField!
    @IBOutlet weak var etIx2Put: UITextField!
    @IBOutlet weak var etIx3Put: UITextField!
    @IBOutlet weak var etARTGet: UITextField!
    @IBOutlet weak var etIx2Get: UITextField!
    @IBOutlet weak var etIx3Get: UITextField!
    @IBOutlet weak var tvOut: UITextView!
    
    var aDataSet: [String] = [String]()
    var aIdxSel: [String] = [String]()
    var aCharSet: [String] = [String]()
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.pvIdxSel3.selectRow(1, inComponent: 0, animated: false)
        self.pvCharSet.selectRow(1, inComponent: 0, animated: false)
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        etARTPut.isEnabled = false
        etIx2Put.isEnabled = false
        etIx3Put.isEnabled = false
        etARTGet.isEnabled = false
        etIx2Get.isEnabled = false
        etIx3Get.isEnabled = false
        tvOut.isEditable = false
        tvOut.text = ""
        etIdxLen.text = "24"
        etCount.text = "1000000"
        etKeyLen.text = "8"
        etValueLen.text = "4"
        self.pvDataSet.delegate = self
        self.pvDataSet.dataSource = self
        self.pvIdxSel2.delegate = self
        self.pvIdxSel2.dataSource = self
        self.pvIdxSel3.delegate = self
        self.pvIdxSel3.dataSource = self
        self.pvCharSet.delegate = self
        self.pvCharSet.dataSource = self
        aDataSet = ["Use generated data", "Million domain names", "DB Pedia Labels"]
        aIdxSel = ["basix", "bfox", "bfqx", "dfox", "dfos", "dfqx", "bft", "dft", "rb_tree"]
        aCharSet = ["Printable", "a to z", "0 to 9", "Octet gap", "", "Random all|", "Seqential all"]
        vc = self
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    @IBAction func btnStart(_ sender: Any) {
        DispatchQueue.global(qos: DispatchQoS.QoSClass.userInteractive).async {
            runNative(Int32(self.pvDataSet.selectedRow(inComponent: 0)),
                      Int32(self.pvIdxSel2.selectedRow(inComponent: 0)),
                      Int32(self.pvIdxSel3.selectedRow(inComponent: 0)),
                      Int32(self.etIdxLen.text!)!,
                      Int(self.etCount.text!)!,
                      Int32(self.pvCharSet.selectedRow(inComponent: 0)) + 1,
                      Int32(self.etKeyLen.text!)!,
                      Int32(self.etValueLen.text!)!,
                      Bundle.main.bundlePath,
                      cb
            )
        }
    }

    func numberOfComponents(in pickerView: UIPickerView) -> Int {
        return 1
    }

    // The number of rows of data
    func pickerView(_ pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        if (pickerView == pvDataSet) {
        return aDataSet.count
        } else if (pickerView == pvIdxSel2 || pickerView == pvIdxSel3) {
        return aIdxSel.count
        } else if (pickerView == pvCharSet) {
        return aCharSet.count
        }
        return 0
    }
    
    // The data to return for the row and component (column) that's being passed in
    func pickerView(_ pickerView: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String? {
        if (pickerView == pvDataSet) {
            return aDataSet[row]
        } else if (pickerView == pvIdxSel2 || pickerView == pvIdxSel3) {
            return aIdxSel[row]
        } else if (pickerView == pvCharSet) {
            return aCharSet[row]
        }
        return ""
    }

    // Catpure the picker view selection
    func pickerView(_ pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
        // This method is triggered whenever the user makes a change to the picker selection.
        // The parameter named row and component represents what was selected.
    }

}
weak var vc: ViewController!

func cb(const_char_star: UnsafePointer<UInt8>!) -> Void{
    var s = String()
    let ptr = const_char_star
    var i8 =  ptr![0]
    var i: Int = 1;
    while (i8 != 0) {
        s.append(Character(UnicodeScalar(i8)))
        i8 = ptr![i]
        i = i + 1
    }
    DispatchQueue.main.async {
        vc.tvOut.text.append(s)
        var s1 = s.range(of: "Get Time: ")
        if (s1 != nil) {
            if (s.starts(with: "ART")) {
                vc.etARTGet.text = String(s.suffix(from: s1!.upperBound))
            } else if (s.starts(with: "Ix1")) {
                vc.etIx2Get.text = String(s.suffix(from: s1!.upperBound))
            } else if (s.starts(with: "Ix2")) {
                vc.etIx3Get.text = String(s.suffix(from: s1!.upperBound))
            }
        }
        s1 = s.range(of: "Insert Time: ")
        if (s1 != nil) {
            if (s.starts(with: "ART")) {
                vc.etARTPut.text = String(s.suffix(from: s1!.upperBound))
            } else if (s.starts(with: "Ix1")) {
                vc.etIx2Put.text = String(s.suffix(from: s1!.upperBound))
            } else if (s.starts(with: "Ix2")) {
                vc.etIx3Put.text = String(s.suffix(from: s1!.upperBound))
            }
        }
    }
}
