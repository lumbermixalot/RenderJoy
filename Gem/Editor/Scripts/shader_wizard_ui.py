"""
Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

import os
import subprocess
import platform
import importlib

from PySide2.QtCore import Qt
from PySide2.QtWidgets import (
    QDialog,
    QLabel,
    QVBoxLayout,
    QLineEdit,
    QTextEdit,
    QPushButton,
    QMessageBox,
)

import shader_wizard
importlib.reload(shader_wizard)
from shader_wizard import ShaderWizard


class ShaderWizardDialog(QDialog):
    def __init__(self, parent=None):
        super(ShaderWizardDialog, self).__init__(parent)

        self.mainLayout = QVBoxLayout(self)
        layout = self.mainLayout
        
        layout.addWidget(QLabel('Shader name'))
        self._shaderNameWidget = QLineEdit()
        layout.addWidget(self._shaderNameWidget)

        self._runButton = QPushButton("Run Wizard")
        self._runButton.clicked.connect(self.on_run_button_clicked)  
        layout.addWidget(self._runButton)

        layout.addWidget(QLabel('Wizard Report'))
        self._reportWidget = QTextEdit()
        layout.addWidget(self._reportWidget)

    def ReportMessage(self, msg: str):
        # append automatically adds a new line
        self._reportWidget.append(msg)

    def OpenPreferredShaderEditor(self, azslFilePath:str):
        if platform.system() == 'Darwin':       # macOS
            subprocess.call(('open', azslFilePath))
        elif platform.system() == 'Windows':    # Windows
            os.startfile(azslFilePath)
        else:                                   # linux variants
            subprocess.call(('xdg-open', azslFilePath))

    def on_run_button_clicked(self):
        shaderName =  self._shaderNameWidget.text()
        wizard = ShaderWizard(reportCB=self)
        #QApplication.setOverrideCursor(Qt.WaitCursor)
        azslFilePath = wizard.Run(shaderName)
        #QApplication.restoreOverrideCursor()
        if azslFilePath is None:
            return
        if not os.path.exists(azslFilePath):
            return
        msgBox = QMessageBox(self)
        msgBox.setWindowTitle("RenderJoy Shader Creation Success")
        msgBox.setText("Do you want to edit the shader file?")
        msgBox.setDetailedText(f"Successfuly created entities, components and shader file:\n{azslFilePath}")
        msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
        msgBox.setIcon(QMessageBox.Question)
        button = msgBox.exec_()
        if button == QMessageBox.Yes:
            self.OpenPreferredShaderEditor(azslFilePath)


if __name__ == "__main__":
    dialog = ShaderWizardDialog()
    dialog.setWindowTitle("RenderJoy Shader Wizard")
    #dialog.resize(240, 240)
    dialog.show()
    dialog.adjustSize()