package com.example.studyproject.data.model

data class Workspace(
    val id: String,
    val name: String,
    val elements: MutableList<BaseElement> = mutableListOf()
)