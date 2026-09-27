import cv2
import numpy as np

# ============================================================
# CONFIG
# ============================================================
PALLET_X1 = 30
PALLET_Y1 = 30
PALLET_X2 = 620
PALLET_Y2 = 530

# กล่องจะ "มืดกว่าพื้นพาเลท" — ถ้าช่องมืดกว่าค่าเฉลี่ยทั้ง ROI เกิน X หน่วย = มีกล่อง
RELATIVE_DARK_OFFSET = 20   # ปรับค่านี้อย่างเดียวพอ (เพิ่ม = เข้มงวดขึ้น)
BOX_RATIO = 0.25            # % pixel ที่มืดกว่า threshold ในช่อง

# ============================================================
cam = cv2.VideoCapture(1)
if not cam.isOpened():
    raise RuntimeError("Cannot open camera!")

while True:
    ret, frame = cam.read()
    if not ret:
        break

    view = frame.copy()
    H, W = view.shape[:2]

    px1, py1 = max(0, PALLET_X1), max(0, PALLET_Y1)
    px2, py2 = min(W, PALLET_X2), min(H, PALLET_Y2)

    pallet_roi = frame[py1:py2, px1:px2]
    pw = px2 - px1
    ph = py2 - py1
    cw = pw // 2
    ch = ph // 2

    # ============================================================
    # KEY: หา threshold แบบ dynamic จาก brightness เฉลี่ยของทั้ง ROI
    # พื้นพาเลทครีม = สว่าง, กล่องเทา = มืดกว่า
    # threshold = mean ทั้ง pallet - offset
    # ============================================================
    gray_pallet = cv2.cvtColor(pallet_roi, cv2.COLOR_BGR2GRAY)
    pallet_mean = np.mean(gray_pallet)
    dynamic_threshold = pallet_mean - RELATIVE_DARK_OFFSET

    # วาดกรอบ + Grid
    cv2.rectangle(view, (px1, py1), (px2, py2), (255, 255, 0), 2)
    cv2.line(view, (px1 + cw, py1), (px1 + cw, py2), (255, 255, 0), 2)
    cv2.line(view, (px1, py1 + ch), (px2, py1 + ch), (255, 255, 0), 2)

    # แสดง dynamic threshold มุมบนซ้าย (ช่วยดีบัก)
    cv2.putText(view, f"Threshold: {dynamic_threshold:.0f} (mean={pallet_mean:.0f})",
                (px1, py1 - 8), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 220, 255), 1)

    box_count = 0

    for r in range(2):
        for c in range(2):
            cx0 = c * cw
            cy0 = r * ch
            cell = gray_pallet[cy0:cy0+ch, cx0:cx0+cw]

            if cell.size == 0:
                continue

            # นับ pixel ที่มืดกว่า dynamic threshold
            dark_pixels = np.sum(cell < dynamic_threshold)
            dark_ratio  = dark_pixels / cell.size

            box_here = dark_ratio > BOX_RATIO
            if box_here:
                box_count += 1

            vx0 = px1 + cx0
            vy0 = py1 + cy0
            vx1 = vx0 + cw
            vy1 = vy0 + ch

            color = (0, 255, 0) if box_here else (0, 0, 255)
            label = "BOX" if box_here else "EMPTY"

            cv2.rectangle(view, (vx0+4, vy0+4), (vx1-4, vy1-4), color, 2)
            cv2.putText(view, label,
                        (vx0 + cw//2 - 40, vy0 + ch//2 + 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.9, color, 2, cv2.LINE_AA)

            # แสดง % และ mean ของช่อง (ช่วยปรับค่า)
            cell_mean = np.mean(cell)
            cv2.putText(view, f"{dark_ratio:.0%} | avg={cell_mean:.0f}",
                        (vx0 + 6, vy1 - 8),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.45, (200, 200, 200), 1)

    cv2.putText(view, f"Boxes: {box_count} / 4",
                (10, H - 15),
                cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 255, 255), 2, cv2.LINE_AA)

    cv2.imshow("Pallet Detection", view)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cam.release()
cv2.destroyAllWindows()