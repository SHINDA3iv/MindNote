package com.example.mindnote

import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.textfield.TextInputEditText

class RegisterActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_register)

        val nameInput = findViewById<TextInputEditText>(R.id.name_input)
        val emailInput = findViewById<TextInputEditText>(R.id.email_input)
        val passwordInput = findViewById<TextInputEditText>(R.id.password_input)
        val registerButton = findViewById<Button>(R.id.register_button)
        val backToLoginButton = findViewById<Button>(R.id.back_to_login_button)

        registerButton.setOnClickListener {
            val name = nameInput.text.toString()
            val email = emailInput.text.toString()
            val password = passwordInput.text.toString()

            if (name.isEmpty() || email.isEmpty() || password.isEmpty()) {
                Toast.makeText(this, "Пожалуйста, заполните все поля", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }

            // TODO: Implement actual registration logic here
            // For now, just simulate successful registration
            Toast.makeText(this, "Регистрация выполнена успешно", Toast.LENGTH_SHORT).show()
            setResult(RESULT_OK)
            finish()
        }

        backToLoginButton.setOnClickListener {
            finish()
        }
    }
} 