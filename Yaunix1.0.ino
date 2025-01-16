#include <Arduino.h>

// ユーザーの情報を保存するためのデータ構造
struct User {
  String username;
  String password;
};

User users[10];  // 最大10人のユーザーをサポート
int userCount = 0;  // 現在のユーザー数
String currentUser = "root";  // 最初はrootユーザー

// コマンド履歴用の配列
String commandHistory[10];
int historyIndex = 0;

// LED点滅パターン
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
  }
}

// コマンド処理
void processCommand(String command) {
  command.trim();  // 余分な空白を取り除く
  if (command.length() == 0) {
    return; // 空のコマンドは無視
  }

  // コマンド履歴に追加
  if (historyIndex < 10) {
    commandHistory[historyIndex] = command;
    historyIndex++;
  } else {
    // 履歴の最初の要素を削除し、新しいコマンドを追加
    for (int i = 0; i < 9; i++) {
      commandHistory[i] = commandHistory[i + 1];
    }
    commandHistory[9] = command;
  }

  blinkLED(1);  // LED点滅

  if (command.startsWith("echo")) {
    // echoコマンド
    String message = command.substring(5);  // "echo "を除いた部分
    Serial.println(message);
  } 
  else if (command.startsWith("user add")) {
    // user addコマンド
    String username = command.substring(9);
    username.trim();
    String password = "";
    Serial.print("Do you want to set a password for " + username + "? (y/n): ");
    while (Serial.available() == 0) { }  // ユーザー入力を待つ
    char answer = Serial.read();
    if (answer == 'y' || answer == 'Y') {
      Serial.print("Enter password for " + username + ": ");
      while (Serial.available() == 0) { }  // ユーザー入力を待つ
      password = Serial.readString();
      
      // パスワード確認
      Serial.print("Re-enter password for confirmation: ");
      String confirmPassword = "";
      while (Serial.available() == 0) { }
      confirmPassword = Serial.readString();

      if (password != confirmPassword) {
        Serial.println("Passwords do not match!");
        return;
      }
    }
    
    // ユーザー情報を登録
    if (userCount < 10) {
      users[userCount].username = username;
      users[userCount].password = password;
      userCount++;
      Serial.println("User " + username + " created.");
    } else {
      Serial.println("User limit reached.");
    }
  } 
  else if (command.startsWith("user del")) {
    // user delコマンド
    String username = command.substring(9);
    username.trim();
    bool found = false;
    for (int i = 0; i < userCount; i++) {
      if (users[i].username == username) {
        // ユーザー削除
        for (int j = i; j < userCount - 1; j++) {
          users[j] = users[j + 1];
        }
        userCount--;
        found = true;
        break;
      }
    }
    if (found) {
      Serial.println("User " + username + " deleted.");
    } else {
      Serial.println("User " + username + " not found.");
    }
  } 
  else if (command.startsWith("su")) {
    // suコマンド
    String targetUser = command.substring(3);
    targetUser.trim();
    if (targetUser == "root") {
      currentUser = "root";  // rootユーザーに戻る
      Serial.println("Switched to user root");
      return;
    }

    bool found = false;
    for (int i = 0; i < userCount; i++) {
      if (users[i].username == targetUser) {
        found = true;
        String inputPassword;
        Serial.print("Enter password for " + targetUser + ": ");
        while (Serial.available() == 0) { }
        inputPassword = Serial.readString();
        inputPassword.trim();  // ここでtrim()を使って無駄な空白を削除
        if (inputPassword == users[i].password) {
          currentUser = targetUser;
          Serial.println("Switched to user " + targetUser);
        } else {
          Serial.println("Incorrect password.");
        }
        break;
      }
    }
    if (!found) {
      Serial.println("User " + targetUser + " not found.");
    }
  }
  else if (command == "help") {
    // helpコマンド
    blinkLED(2);  // LED点滅
    Serial.println("Available commands:");
    Serial.println("1. echo <message> : Prints the message to the screen.");
    Serial.println("2. user add <username> : Adds a new user.");
    Serial.println("   You will be prompted to set a password.");
    Serial.println("3. user del <username> : Deletes a user.");
    Serial.println("4. su <username> : Switches to the specified user.");
    Serial.println("   You will be prompted for the password if one is set.");
    Serial.println("5. help : Displays this help message.");
  }
  else {
    blinkLED(3);  // LED点滅
    Serial.println("Unknown command: " + command);
  }
}

void setup() {
  // シリアル通信を初期化
  Serial.begin(9600);
  while (!Serial) { }

  // 13番ピンをLEDとして使用する設定
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);  // 初期状態でLEDを消灯

  Serial.println("Yaunix 1.0 chatgpt created");
  Serial.println("Welcome! Type a command:");

  // 最初にプロンプトを表示
  Serial.print(currentUser + "@yaunix: # ");
}

void loop() {
  // シリアルモニタからの入力があった場合のみプロンプトを表示
  if (Serial.available() > 0) {
    // シリアルモニタからの入力を処理
    String command = Serial.readString();
    processCommand(command);

    // コマンド実行後に再度プロンプトを表示
    Serial.print(currentUser + "@yaunix: # ");
  }
}
