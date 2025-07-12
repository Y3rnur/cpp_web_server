function sayHi() {
    alert("Hello from Javascript!");
    console.log('The "Click me" button was pressed!');
}

function updateClock() {
    const now = new Date;   // Variable for getting current date and time
    const options = {hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false};
    const formattedTime = now.toLocaleTimeString('en-US', options);

    const clockElement = document.getElementById('client-clock');
    if (clockElement) {
        clockElement.textContent = `Client Time: ${formattedTime}`;     // Updating the time
    }
}

function updateCharactersCount() {
    const messageTextarea = document.getElementById('user_message');
    const charCountElement = document.getElementById('char-count');     // Getting IDs of textarea and display element

    if (messageTextarea && charCountElement) {
        const currentCount = messageTextarea.value.length;  // Getting the number of characters
        charCountElement.textContent = `Characters: ${currentCount}`;   // Updating the number of characters
    }
}

document.addEventListener('DOMContentLoaded', () => {

    const clockElement = document.getElementById('client-clock');
    if (clockElement) {
        updateClock();
        setInterval(updateClock, 1000);     // Updating every second
    }

    const messageTextarea = document.getElementById('user_message');
    if (messageTextarea) {
        messageTextarea.addEventListener('input', updateCharactersCount);
        updateCharactersCount();     // After every typed character by user, we update char. count
    }
});