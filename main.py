import sys
import typing as T
from pathlib import Path
from datetime import datetime

from PySide6.QtWidgets import (
    QMainWindow,
    QApplication,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
    QSizePolicy,
    QLineEdit,
    QDialog,
    QDialogButtonBox,
    QGridLayout,
    QSpinBox,
)
from PySide6.QtGui import (
    QCloseEvent,
    QColor,
    QPainter,
    QBrush,
    QPen,
    QPolygon,
    QPixmap,
    QAction,
    QGuiApplication,
)
from PySide6.QtCore import Qt, QTimer, QTime, QPoint, QEvent, QSize, QSettings
import pandas as pd


class DescriptionDialog(QDialog):
    def __init__(self, parent: T.Optional[QWidget] = None):
        super().__init__(parent)

        self.setWindowFlag(Qt.FramelessWindowHint)
        self.setFixedSize(300, 100)

        self.description = QLineEdit(self)
        self.description.setPlaceholderText("Enter description")
        self.description.setMinimumHeight(40)
        self.description.setFocus()

        self.button_box = QDialogButtonBox(QDialogButtonBox.Save)
        self.button_box.accepted.connect(self.accept)

        layout = QVBoxLayout()
        layout.addWidget(self.description)
        layout.addWidget(self.button_box)
        self.setLayout(layout)

    def accept(self):
        super().accept()


class SettingsDialog(QDialog):
    def __init__(self, settings: QSettings, parent: T.Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self.settings = settings

        self.setWindowFlag(Qt.FramelessWindowHint)
        self.setFixedSize(300, 180)

        layout = QGridLayout()
        self.setLayout(layout)

        default_path = Path(self.settings.fileName()).resolve().parent / datetime.now().strftime(
            "%Y-%m-%d_%H:%M:%S"
        )
        self.file_path_label = QLabel("File path")
        self.file_path = QLineEdit(self.settings.value("FilePath", default_path.as_posix()))
        self.file_path.setFocus()
        layout.addWidget(self.file_path_label, 0, 0)
        layout.addWidget(self.file_path, 0, 1)

        self.update_database_interval_label = QLabel("Screenshot")
        self.screen_shot_interval = QSpinBox()
        self.screen_shot_interval.setMinimum(1)
        self.screen_shot_interval.setMaximum(60)
        self.screen_shot_interval.setValue(int(self.settings.value("ScreenShotInterval", 30)))
        layout.addWidget(self.update_database_interval_label, 1, 0)
        layout.addWidget(self.screen_shot_interval, 1, 1)

        self.update_tracking_interval_label = QLabel("Tracking")
        self.tracking_interval = QSpinBox()
        self.tracking_interval.setMinimum(1)
        self.tracking_interval.setMaximum(60)
        self.tracking_interval.setValue(int(self.settings.value("TrackingInterval", 1)))
        layout.addWidget(self.update_tracking_interval_label, 2, 0)
        layout.addWidget(self.tracking_interval, 2, 1)

        self.button_box = QDialogButtonBox(QDialogButtonBox.Save | QDialogButtonBox.Cancel)
        self.button_box.accepted.connect(self.accept)
        self.button_box.rejected.connect(self.reject)
        layout.addWidget(self.button_box, 3, 0, 1, 2)

    def accept(self) -> None:
        self.settings.setValue("FilePath", self.file_path.text())
        self.settings.setValue("ScreenShotInterval", self.screen_shot_interval.value())
        self.settings.setValue("TrackingInterval", self.tracking_interval.value())
        super().accept()

    def reject(self) -> None:
        if self.settings.value("FilePath") is None:
            self.settings.setValue("FilePath", self.file_path.text())
        if self.settings.value("ScreenShotInterval") is None:
            self.settings.setValue("ScreenShotInterval", self.screen_shot_interval.value())
        if self.settings.value("TrackingInterval") is None:
            self.settings.setValue("TrackingInterval", self.tracking_interval.value())
        super().reject()


class TimeTracker(QMainWindow):
    def __init__(self) -> None:
        super().__init__()

        # setting window title
        self.setWindowTitle("Time Tracker")
        self.setFixedSize(300, 300 + 50)
        self.has_not_started_color = QColor(168, 160, 164, 0.8)
        self.has_started_color = QColor(230, 229, 229, 0.8)
        # self.setStyleSheet(f"background-color: {self.has_not_started_color.name()};")

        self.start_time = QTime.currentTime()
        self.settings = QSettings()
        self.settings_dialog = SettingsDialog(self.settings, self)
        if (
            self.settings.value("FilePath") is None
            or self.settings.value("ScreenShotInterval") is None
            or self.settings.value("TrackingInterval") is None
        ):
            self.popup_settings()
        self.file_path = self.settings.value("FilePath")
        self.screen_shot_interval = int(self.settings.value("ScreenShotInterval"))
        self.tracking_interval = int(self.settings.value("TrackingInterval"))

        # Current working time when the app starts
        self.current_working_label = QLabel("00:00", self)  # hours:minutes
        self.current_working_label.setStyleSheet("background-color: rgba(255, 255, 255, 0);")
        self.current_working_label.setStyleSheet("font-size: 30px")
        self.current_working_label.move(110, 80)

        # Total working time including previous working time
        self.prev_working_hours, self.prev_working_minutes = self.get_previous_working_time()
        self.total_working_label = QLabel("", self)  # hours:minutes
        self.total_working_label.setStyleSheet("background-color: rgba(255, 255, 255, 0);")
        self.total_working_label.setText(f"{self.prev_working_hours:02d}:{self.prev_working_minutes:02d}")
        self.total_working_label.setStyleSheet("font-size: 20px")
        self.total_working_label.move(122, 250)

        # Settings
        self.menu = self.menuBar().addMenu("File")
        self.settings_action = QAction("Settings", self)
        self.settings_action.setShortcut(Qt.CTRL | Qt.Key_QuoteLeft)
        self.menu.addAction(self.settings_action)
        self.settings_action.triggered.connect(self.popup_settings)

        self.stop_action = QAction("Stop", self)
        self.stop_action.setShortcut(Qt.CTRL | Qt.Key_S)
        self.stop_action.setDisabled(True)
        self.menu.addAction(self.stop_action)
        self.stop_action.triggered.connect(self.stop_tracking)
        self.description_dialog = DescriptionDialog(self)

        self.exit_action = QAction("Exit", self)
        self.exit_action.setShortcut(Qt.CTRL | Qt.Key_Q)
        self.menu.addAction(self.exit_action)
        self.exit_action.triggered.connect(self.close)

        # Start button
        pixmap = QPixmap(Path(__file__).resolve().parent.as_posix() + "/assets/start.png")

        self.start = QPushButton(self)
        self.start.setIcon(pixmap)
        self.start.setIconSize(QSize(120, 120))
        x = 90
        self.start.setGeometry(x, x, self.width() - 2 * x, self.height() - 2 * x)
        self.start.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.start.setStyleSheet("QPushButton { border-radius: 60px; background-color: transparent}")
        self.start.clicked.connect(self.start_tracking)

        # Clock update timer => call PaintEvent every second
        timer = QTimer(self)
        timer.start(1000)
        timer.timeout.connect(self.update)

        # Update current working
        self.current_working_timer = QTimer(self)
        self.current_working_timer.timeout.connect(self.update_working_time)

        # Update databse timer
        self.screenshot_timer = QTimer(self)
        self.screenshot_timer.timeout.connect(self.screenshot)

        # hour hand
        self.hour_hand = QPolygon([QPoint(6, 7), QPoint(-6, 7), QPoint(0, -50)])
        self.hour_hand_color = Qt.black
        # minute hand
        self.minute_hand = QPolygon([QPoint(6, 7), QPoint(-6, 7), QPoint(0, -70)])
        self.minute_hand_color = Qt.black
        # second hand
        self.second_hand = QPolygon([QPoint(1, 1), QPoint(-1, 1), QPoint(0, -90)])
        self.second_hand_color = Qt.red

    def popup_settings(self) -> None:
        self.settings_dialog.exec()

        # update the settings
        self.file_path = self.settings.value("FilePath")
        self.screen_shot_interval = int(self.settings.value("ScreenShotInterval"))
        self.tracking_interval = int(self.settings.value("TrackingInterval"))

        # Update screen accordingly
        self.current_working_label.setText("00:00")  # to update settings, need to stop the tracking
        self.prev_working_hours, self.prev_working_minutes = self.get_previous_working_time()
        self.total_working_label.setText(f"{self.prev_working_hours:02d}:{self.prev_working_minutes:02d}")

    def start_tracking(self) -> None:
        # Hide the start button
        self.start.hide()
        self.start.setDisabled(True)

        # Disable settings during tracking
        self.settings_action.setDisabled(True)
        # Enable stop action
        self.stop_action.setEnabled(True)

        self.current_working_timer.start(self.tracking_interval * 1000 * 60)
        self.screenshot_timer.start(self.screen_shot_interval * 1000 * 60)

        data_file = Path(self.file_path + ".csv")
        if not data_file.exists():
            data = pd.DataFrame(columns=["start_time", "end_time", "description"])
            data.to_csv(data_file, index=False)
        data = pd.read_csv(data_file)
        start_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        end_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        description = ""
        data.loc[len(data)] = [start_time, end_time, description]
        data.to_csv(data_file, index=False)

        # Capture a screenshot
        self.screenshot()

        self.start_time = QTime.currentTime()
        # self.setStyleSheet(f"background-color: {self.has_started_color.name()};")

    def update_working_time(self) -> None:
        """While doing work, update the current working time."""
        current = QTime.currentTime()
        seconds = self.start_time.secsTo(current)

        ch = seconds // 3600
        cm = (seconds % 3600) // 60
        self.current_working_label.setText(f"{ch:02d}:{cm:02d}")

        h = self.prev_working_hours + ch
        m = self.prev_working_minutes + cm
        h += m // 60
        m %= 60
        self.total_working_label.setText(f"{h:02d}:{m:02d}")

        # Only update end time in the database
        data = pd.read_csv(self.file_path + ".csv")
        data.loc[data.index[-1], "end_time"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        data.to_csv(self.file_path + ".csv", index=False)

    def screenshot(self) -> None:
        name = datetime.now().strftime("%Y-%m-%d_%H:%M:%S") + ".png"
        # screenshot = QApplication.primaryScreen().grabWindow(0)
        screen = QGuiApplication.primaryScreen()
        window = self.windowHandle()
        if window:
            screen = window.screen()
        if not screen:
            print("No screen found")
            return
        screenshot = screen.grabWindow(0)
        Path(self.file_path).mkdir(parents=True, exist_ok=True)
        screenshot.save(self.file_path + "/" + name, "png")

    def get_previous_working_time(self) -> T.Tuple[int, int]:
        data_file = Path(self.file_path + ".csv")
        if not data_file.exists():
            data = pd.DataFrame(columns=["start_time", "end_time", "description"])
            data.to_csv(data_file, index=False)
        data = pd.read_csv(data_file)
        prev_working_hours = 0
        prev_working_minutes = 0
        for _, row in data.iterrows():
            start_time = datetime.strptime(row["start_time"], "%Y-%m-%d %H:%M:%S")
            end_time = datetime.strptime(row["end_time"], "%Y-%m-%d %H:%M:%S")
            prev_working_hours += (end_time - start_time).seconds // 3600
            prev_working_minutes += ((end_time - start_time).seconds % 3600) // 60
        prev_working_hours += prev_working_minutes // 60
        prev_working_minutes %= 60
        return prev_working_hours, prev_working_minutes

    def stop_tracking(self) -> None:
        # Stop the timers
        self.current_working_timer.stop()
        self.screenshot_timer.stop()

        # Enable start button
        self.start.setEnabled(True)
        self.start.show()
        # Disable stop action
        self.stop_action.setDisabled(True)
        # Enable settings
        self.settings_action.setEnabled(True)

        self.description_dialog.exec()
        description = self.description_dialog.description.text()
        data = pd.read_csv(self.file_path + ".csv")
        data.loc[data.index[-1], "description"] = description
        data.to_csv(self.file_path + ".csv", index=False)

        # Capture a screenshot
        self.screenshot()

        # Update label
        self.current_working_label.setText("00:00")  # to update settings, need to stop the tracking
        self.prev_working_hours, self.prev_working_minutes = self.get_previous_working_time()
        self.total_working_label.setText(f"{self.prev_working_hours:02d}:{self.prev_working_minutes:02d}")
        # self.setStyleSheet(f"background-color: {self.has_not_started_color.name()};")
        self.description_dialog.description.clear()

    def paintEvent(self, event: QEvent) -> None:
        # make a square
        rec = min(self.width(), self.height())

        # getting current time
        current = QTime.currentTime()

        # creating a painter object
        painter = QPainter(self)

        # method to draw the hands
        # argument : color rotation and which hand should be pointed
        def drawPointer(color, rotation, pointer):

            # setting brush
            painter.setBrush(QBrush(color))

            # saving painter
            painter.save()

            # rotating painter
            painter.rotate(rotation)

            # draw the polygon i.e hand
            painter.drawConvexPolygon(pointer)

            # restore the painter
            painter.restore()

        # tune up painter
        painter.setRenderHint(QPainter.Antialiasing)
        painter.translate(self.width() / 2, self.height() / 2)
        painter.scale(rec / 200, rec / 200)
        painter.setPen(Qt.NoPen)

        # draw each hand
        drawPointer(self.hour_hand_color, (30 * (current.hour() + current.minute() / 60)), self.hour_hand)
        drawPointer(
            self.minute_hand_color, (6 * (current.minute() + current.second() / 60)), self.minute_hand
        )
        drawPointer(self.second_hand_color, (6 * current.second()), self.second_hand)

        # drawing background
        painter.setPen(QPen(Qt.green))

        for i in range(0, 60):
            # drawing background lines
            if i % 5 == 0:
                # Black color for hours
                painter.setPen(QPen(Qt.black))
                painter.drawLine(87, 0, 97, 0)
            else:
                # Gray color for minutes
                painter.setPen(QPen(Qt.gray))
                painter.drawLine(93, 0, 97, 0)
            painter.rotate(6)

        # ending the painter
        painter.end()

    def closeEvent(self, event: QCloseEvent) -> None:
        if self.current_working_timer.isActive():
            self.stop_tracking()
        return super().closeEvent(event)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setOrganizationName("TimeTracker")
    app.setApplicationName("TimeTracker")
    win = TimeTracker()
    win.show()
    sys.exit(app.exec())
