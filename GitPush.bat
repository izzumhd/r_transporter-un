@echo off
echo ===============================
echo   Update dan Push ke GitHub 🚀
echo ===============================

cd /d "D:\P codeng\GitHub\r_transporter_un"

echo 🔁 Menarik update dari GitHub dulu...
git pull origin main

echo ➕ Mengecek perubahan lokal...
git add .

rem cek apakah ada perubahan yang perlu di-commit
git diff --cached --quiet
if %errorlevel%==0 (
    echo ⚡ Tidak ada perubahan untuk di-commit.
) else (
    echo 💬 Commit perubahan...
    git commit -m "update dari folder lokal"

    echo 🚀 Push ke GitHub...
    git push origin main
)

echo.
echo ✅ Selesai! Semua udah sinkron 😎
pause
