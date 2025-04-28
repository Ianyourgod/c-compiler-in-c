int callee() {
    int times_called = 0;
    return ++times_called;
}

int main(void) {
    for (int i=0;i<99;i++) {
        callee();
    }
    return callee();
}