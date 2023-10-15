
// Fetch data from ThingSpeak using their API
const channelId = 2304789; // Replace with your ThingSpeak channel ID
const apiKey = "JEBT4LJFFX2KLEV8"; // Replace with your ThingSpeak API key

const url = `https://api.thingspeak.com/channels/${channelId}/feeds.json?api_key=${apiKey}&results=2`;

function fetchData() {
    fetch(url)
        .then((response) => response.json())
        .then((data) => {
            const tbody = document.getElementById("dataBody");
            tbody.innerHTML = ""; // Clear previous data

            generateChart(data.feeds);

            data.feeds.forEach((feed) => {
                const field1Json = JSON.parse(feed.field2);

                const present = [field1Json.enrollment];
                const row = `
            <tr>
              <td>${present.join("<br>")}</td>
              <td>${field1Json.start1} ${field1Json.start2}</td>
              <td>${field1Json.end1} ${field1Json.end2}</td>
              <td>${field1Json.degree}</td>
              <td>${field1Json.branch}</td>
              <td>${field1Json.sem}</td>
              <td>${field1Json.sub}</td>
            </tr>
          `;

                tbody.innerHTML += row;
                var presentEnrollments = field1Json.enrollment;
                
            });

        })
        .catch((error) => console.error("Error:", error));
}

let myChart = null; // Keep track of the chart instance

function generateChart(data) {
    if (myChart !== null) {
        myChart.destroy(); // Destroy previous chart
    }

    const enrollments = data.map(feed => JSON.parse(feed.field2).enrollment);
    const counts = {};

    enrollments.forEach(enrollment => {
        counts[enrollment] = counts[enrollment] ? counts[enrollment] + 1 : 1;
    });

    const ctx = document.getElementById('enrollmentChart').getContext('2d');
    myChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: Object.keys(counts),
            datasets: [{
                label: 'Enrollment Counts',
                data: Object.values(counts),
                backgroundColor: 'rgba(75, 192, 192, 0.2)',
                borderColor: 'rgba(75, 192, 192, 1)',
                borderWidth: 1
            }]
        },
        options: {
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Enrollment'
                    },
                    beginAtZero: true
                },
                y: {
                    title: {
                        display: true,
                        text: 'Count'
                    },
                    beginAtZero: true
                }
            }
        }
    });
}
