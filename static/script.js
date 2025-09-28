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

document.addEventListener('DOMContentLoaded', function() {
    // Add notification div if not present
    if (!document.getElementById('notification')) {
        const notif = document.createElement('div');
        notif.id = 'notification';
        notif.style.position = 'fixed';
        notif.style.top = '30px';
        notif.style.left = '50%';
        notif.style.transform = 'translateX(-50%)';
        notif.style.background = '#4CAF50';
        notif.style.color = 'white';
        notif.style.padding = '16px 32px';
        notif.style.borderRadius = '8px';
        notif.style.fontSize = '1.2em';
        notif.style.boxShadow = '0 2px 8px rgba(0,0,0,0.15)';
        notif.style.display = 'none';
        notif.style.zIndex = '9999';
        document.body.appendChild(notif);
    }

    function showNotification(msg) {
        const notif = document.getElementById('notification');
        notif.textContent = msg;
        notif.style.display = 'block';
        setTimeout(() => {
            notif.style.display = 'none';
        }, 2500);
    }

    document.querySelectorAll('.delete-button').forEach(function(button) {
        button.addEventListener('click', function(e) {
            const noteDiv = button.closest('.note');
            const noteId = noteDiv.getAttribute('data-note-id');
            if (!noteId) return;
            if (confirm('Are you sure to delete the given message? This process is irreversible.')) {
                fetch('/delete_note', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded'
                    },
                    body: 'id=' + encodeURIComponent(noteId)
                })
                .then(response => {
                    if (response.ok) {
                        noteDiv.remove();
                        showNotification('Message was deleted successfully!');
                    } else {
                        showNotification('Failed to delete the note.');
                    }
                })
                .catch(() => showNotification('Error communicating with the server.'));
            }
        });
    });
});