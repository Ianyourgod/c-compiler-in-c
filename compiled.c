int other_function() {
    int a = 5;
    return a + 1;
}

int main(void) {
    int a = other_function();
    return a*5;
}