var githubApiBase = "{{ site.github.api_url }}";
var githubApiRepos = githubApiBase + "/repos/{{ site.github.owner_name }}/{{ site.github.project_title }}";

// The GitHub API has a rate limit of 60 requests per hour for anonymous requests
// This makes use of the local storage to make sure only 1 request is sent per hour
function getLatestRelease(callback) {
    if (Storage) {
        var storageData = null;
        try {
            if (localStorage.latest_release) {
                storageData = JSON.parse(localStorage.latest_release);
            }
        } catch (e) { }
        if (!storageData || storageData.updated < (new Date().getTime() / 1000) - (60 * 60)) {
            return refreshLatestRelease(function (data) {
                localStorage.latest_release = JSON.stringify({
                    release: data,
                    updated: new Date().getTime() / 1000
                });
                callback(data);
            });
        } else {
            return callback(storageData.release);
        }
    } else {
        return refreshLatestRelease(callback);
    }
}

function refreshLatestRelease(callback) {
    $.getJSON(githubApiRepos + "/releases/latest")
        .done(callback)
        .fail(function (xhr, status, error) { callback(null); });
}
