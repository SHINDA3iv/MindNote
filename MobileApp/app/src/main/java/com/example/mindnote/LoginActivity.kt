package com.example.mindnote

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.textfield.TextInputEditText

class LoginActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_login)

        val emailInput = findViewById<TextInputEditText>(R.id.email_input)
        val passwordInput = findViewById<TextInputEditText>(R.id.password_input)
        val loginButton = findViewById<Button>(R.id.login_button)
        val registerButton = findViewById<Button>(R.id.register_button)
        val guestButton = findViewById<Button>(R.id.guest_button)

        loginButton.setOnClickListener {
            val email = emailInput.text.toString()
            val password = passwordInput.text.toString()

            if (email.isEmpty() || password.isEmpty()) {
                Toast.makeText(this, "Пожалуйста, заполните все поля", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }

            // TODO: Implement actual login logic here
            // For now, just show a message
            Toast.makeText(this, "Функция входа будет доступна позже", Toast.LENGTH_SHORT).show()
        }

        registerButton.setOnClickListener {
            val intent = Intent(this, RegisterActivity::class.java)
            startActivity(intent)
        }

        guestButton.setOnClickListener {
            val resultIntent = Intent()
            resultIntent.putExtra("is_guest", true)
            setResult(RESULT_OK, resultIntent)
            finish()
        }
    }
} 