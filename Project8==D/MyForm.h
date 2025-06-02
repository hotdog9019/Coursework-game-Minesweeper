#pragma once
using namespace System;
using namespace System::IO;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace System::Collections::Generic;

public ref class MyForm : public Form
{
private:
    PictureBox^ gameOverGif;
    Panel^ menuPanel;
    Button^ playButton;
    Button^ exitButton;
    Label^ highScoreLabel;
    array<Button^>^ buttons;
    int fieldSize = 8;
    List<int>^ bombIndices;
    int bombCount = 10;
    int score = 0;
    int highScore = 0;
    Button^ showBombBtn;
    Button^ doublePointsBtn;
    bool bombShown = false;
    bool doublePoints = false;
    Label^ scoreLabel;
    String^ highScoreFile = Path::Combine(Application::StartupPath, "highscore.txt");
    Image^ mineImage = Image::FromFile(Path::Combine(Application::StartupPath, "mine.png"));
    Image^ flagImage = Image::FromFile(Path::Combine(Application::StartupPath, "flag.png"));

public:
    MyForm()
    {
        this->Text = "Минёр";
        this->MaximizeBox = false;
        this->ClientSize = Drawing::Size(fieldSize * 40 + 20, fieldSize * 40 + 130);
        this->BackColor = Color::White;
        this->FormBorderStyle = Windows::Forms::FormBorderStyle::FixedSingle;
        LoadHighScore();
        InitializeMenu();
    }

private:
    void InitializeMenu()
    {
        menuPanel = gcnew Panel();
        menuPanel->Dock = DockStyle::Fill;

        playButton = gcnew Button();
        playButton->Text = "Играть";
        playButton->Size = Drawing::Size(200, 50);
        playButton->Location = Point((ClientSize.Width - 200) / 2, 50);
        playButton->Click += gcnew EventHandler(this, &MyForm::StartGame);
        menuPanel->Controls->Add(playButton);

        exitButton = gcnew Button();
        exitButton->Text = "Выход";
        exitButton->Size = Drawing::Size(200, 50);
        exitButton->Location = Point((ClientSize.Width - 200) / 2, 120);
        exitButton->Click += gcnew EventHandler(this, &MyForm::OnExit);
        menuPanel->Controls->Add(exitButton);

        highScoreLabel = gcnew Label();
        highScoreLabel->ForeColor = Color::DarkSlateGray;
        highScoreLabel->Font = gcnew Drawing::Font("Arial", 12, FontStyle::Bold);
        highScoreLabel->AutoSize = true;
        highScoreLabel->Location = Point((ClientSize.Width - 150) / 2, 190);
        menuPanel->Controls->Add(highScoreLabel);
        UpdateHighScoreLabel();

        this->Controls->Add(menuPanel);
    }

    void InitializeGameComponents(bool visible)
    {
        buttons = gcnew array<Button^>(fieldSize * fieldSize);
        bombIndices = gcnew List<int>();
        Random^ rnd = gcnew Random();
        Controls->Clear();
        Controls->Add(menuPanel);

        while (bombIndices->Count < bombCount) {
            int index = rnd->Next(fieldSize * fieldSize);
            if (!bombIndices->Contains(index)) bombIndices->Add(index);
        }

        for (int i = 0; i < buttons->Length; i++) {
            Button^ btn = gcnew Button();
            btn->Size = Drawing::Size(40, 40);
            btn->Location = Point((i % fieldSize) * 40 + 10, (i / fieldSize) * 40 + 10);
            btn->Tag = i;
            btn->MouseDown += gcnew MouseEventHandler(this, &MyForm::OnCellMouseDown);
            btn->Font = gcnew Drawing::Font("Consolas", 12, FontStyle::Bold);
            btn->Visible = visible;
            this->Controls->Add(btn);
            buttons[i] = btn;
        }

        scoreLabel = gcnew Label();
        scoreLabel->Text = "Счёт: 0";
        scoreLabel->ForeColor = Color::Black;
        scoreLabel->Location = Point(10, fieldSize * 40 + 20);
        scoreLabel->AutoSize = true;
        scoreLabel->Visible = visible;
        this->Controls->Add(scoreLabel);

        showBombBtn = gcnew Button();
        showBombBtn->Text = "Показать бомбу";
        showBombBtn->Size = Drawing::Size(120, 30);
        showBombBtn->Location = Point(10, fieldSize * 40 + 50);
        showBombBtn->Click += gcnew EventHandler(this, &MyForm::OnShowBomb);
        showBombBtn->Visible = visible;
        this->Controls->Add(showBombBtn);

        doublePointsBtn = gcnew Button();
        doublePointsBtn->Text = "х2 очки";
        doublePointsBtn->Size = Drawing::Size(100, 30);
        doublePointsBtn->Location = Point(140, fieldSize * 40 + 50);
        doublePointsBtn->Click += gcnew EventHandler(this, &MyForm::OnDoublePoints);
        doublePointsBtn->Visible = visible;
        this->Controls->Add(doublePointsBtn);


        gameOverGif = gcnew PictureBox();
        gameOverGif->Image = Image::FromFile("C:/Users/tema/Downloads/techies-с.gif");
        gameOverGif->SizeMode = PictureBoxSizeMode::StretchImage;
        gameOverGif->Left = 0;
        gameOverGif->Top = 0;
        gameOverGif->Width = fieldSize * 40 + 20;
        gameOverGif->Height = fieldSize * 40 + 20;
        gameOverGif->Visible = false;
        this->Controls->Add(gameOverGif);
        gameOverGif->BringToFront();

    }

    void StartGame(Object^, EventArgs^)
    {
        menuPanel->Visible = false;
        score = 0;
        doublePoints = false;
        bombShown = false;

        InitializeGameComponents(true);
    }


    void OnExit(Object^, EventArgs^) {
        Application::Exit();
    }

    void OnCellMouseDown(Object^ sender, MouseEventArgs^ e)
    {
        Button^ btn = safe_cast<Button^>(sender);
        int index = safe_cast<int>(btn->Tag);

        if (e->Button == Windows::Forms::MouseButtons::Right) {
            if (btn->Image == flagImage) {
                btn->Image = nullptr;
            }
            else {
                btn->Image = flagImage;
            }

            return;
        }

        RevealCell(index);
    }

    void RevealCell(int index)
    {
        if (index < 0 || index >= buttons->Length) return;
        Button^ btn = buttons[index];
        if (!btn->Enabled || btn->Text == "🚩") return;

        if (bombIndices->Contains(index)) {
            btn->Image = mineImage;
            GameOver(false);
            return;
        }

        int row = index / fieldSize, col = index % fieldSize, count = 0;
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                int r = row + dr, c = col + dc;
                if (r >= 0 && r < fieldSize && c >= 0 && c < fieldSize) {
                    int i = r * fieldSize + c;
                    if (bombIndices->Contains(i)) count++;
                }
            }
        }

        btn->Enabled = false;
        btn->BackColor = Color::LightGray;
        if (count > 0) {
            btn->Text = count.ToString();
            btn->ForeColor = GetNumberColor(count);
        }

        int gained = doublePoints ? 2 : 1;
        score += gained;
        scoreLabel->Text = "Счёт: " + score;

        if (CheckWin()) GameOver(true);

        if (count == 0) {
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    int r = row + dr, c = col + dc;
                    if (r >= 0 && r < fieldSize && c >= 0 && c < fieldSize) {
                        RevealCell(r * fieldSize + c);
                    }
                }
            }
        }
    }

    void GameOver(bool win)
    {
        for each(Button ^ b in buttons) b->Enabled = false;

        String^ msg;
        if (win) {
            msg = "Победа! Очки: " + score;
            if (score > highScore) {
                highScore = score;
                SaveHighScore();
                UpdateHighScoreLabel();
                msg += "\nНовый рекорд!";
            }
        }
        else {
            msg = "Бум! Проигрыш.";
        }

        MessageBox::Show(msg, "Итог", MessageBoxButtons::OK);
        Application::Restart();
    }

    bool CheckWin()
    {
        for (int i = 0; i < buttons->Length; i++) {
            if (!bombIndices->Contains(i) && buttons[i]->Enabled) {
                return false;
            }
        }
        return true;
    }

    Color GetNumberColor(int number) {
        array<Color>^ colors = {
            Color::Blue, Color::Green, Color::Red, Color::Navy,
            Color::Maroon, Color::Turquoise, Color::Black, Color::Gray
        };
        return colors[number - 1];
    }

    void OnShowBomb(Object^, EventArgs^) {
        if (!bombShown) {
            Random^ rnd = gcnew Random();
            int idx = bombIndices[rnd->Next(bombIndices->Count)];
            buttons[idx]->BackColor = Color::Yellow;
            bombShown = true;
            showBombBtn->Enabled = false;
        }
    }

    void OnDoublePoints(Object^, EventArgs^) {
        doublePoints = true;
        doublePointsBtn->Enabled = false;
    }

    void LoadHighScore() {
        try {
            if (File::Exists(highScoreFile)) {
                highScore = Int32::Parse(File::ReadAllText(highScoreFile));
            }
        }
        catch (...) {
            highScore = 0;
        }
        UpdateHighScoreLabel();
    }

    void SaveHighScore() {
        try {
            File::WriteAllText(highScoreFile, highScore.ToString());
        }
        catch (...) {}
    }

    void UpdateHighScoreLabel() {
        if (highScoreLabel != nullptr)
            highScoreLabel->Text = "Рекорд: " + highScore.ToString();
    }
};
