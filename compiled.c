int other_function(int j) {
    int a = 5;
    return a * j;
}

int main(void) {
    int a = other_function(5);
    return a*5;
}