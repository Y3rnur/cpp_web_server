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

const quotes = [
    {quote: "The only way to do great work is to love what you do.", author: "Steve Jobs"},
    {quote: "Believe you can and you're halfway there.", author: "Theodore Roosevelt"},
    {quote: "The future belongs to those who believe in the beauty of their dreams.", author: "Eleanor Roosevelt"},
    {quote: "Strive not to be a success, but rather to be of value.", author: "Albert Einstein"},
    {quote: "The best way to predict the future is to create it.", author: "Peter Drucker"},
    {quote: "It always seems impossible until it's done.", author: "Nelson Mandela"}
];

function displayRandomQuote() {
    const quoteDisplayElement = document.getElementById('quote-display');
    if (quoteDisplayElement) {
        const randomIndex = Math.floor(Math.random() * quotes.length);  // Getting random index
        const randomQuote = quotes[randomIndex];    // Getting random quote object

        quoteDisplayElement.innerHTML = `"${randomQuote.quote}" - <strong>${randomQuote.author}</strong>`;
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

    const newQuoteButton = document.getElementById('newQuoteBtn');
    if (newQuoteButton) {
        newQuoteButton.addEventListener('click', displayRandomQuote);
        displayRandomQuote();   // Calling once the button is clicked
    }
});