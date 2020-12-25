import * as core from "@actions/core";
import * as github from "@actions/github";

async function run() {
  const token = core.getInput("token");

  const octokit = new github.GitHub(token);
  const context = github.context;

  const comments = await octokit.issues.listComments({
    ...context.repo,
    issue_number: context.payload.pull_request!.number
  });

  const pull_request = await octokit.pulls.get({
    ...context.repo,
    pull_number: context.payload.pull_request!.number
  });

  const actionComment = comments.data.find(
      comment => comment.body.indexOf("Package: ") >= 0 &&
          comment.body.indexOf("Version: ") >= 0 &&
          comment.body.indexOf("Action: ") >= 0 &&
          comment.body.indexOf("Artifact: ") >= 0 &&
          comment.body.indexOf("Commit: ") >= 0
  );

  if (actionComment) {
    let pack = actionComment!.body.substr(actionComment!.body.indexOf("Package: ") + 9)
    pack = pack.substr(0, pack.indexOf("\n"));
    let version = actionComment!.body.substr(actionComment!.body.indexOf("Version: ") + 9)
    version = version.substr(0, version.indexOf("\n"));
    let action = actionComment!.body.substr(actionComment!.body.indexOf("Action: ") + 8)
    action = action.substr(0, action.indexOf("\n"));
    let artifact = actionComment!.body.substr(actionComment!.body.indexOf("Artifact: ") + 10)
    artifact = artifact.substr(0, artifact.indexOf("\n"));
    let commit = actionComment!.body.substr(actionComment!.body.indexOf("Commit: ") + 8)

    core.setOutput(
        "publish",
        pull_request.data.labels
            .some(label => label.name === core.getInput("label"))
            .toString()
    );
    core.setOutput("package", pack.trim());
    core.setOutput("version", version.trim());
    core.setOutput("action", action.trim());
    core.setOutput("artifact", artifact.trim());
    core.setOutput("commit", commit.trim());

    console.log("package", pack.trim());
    console.log("version", version.trim());
    console.log("action", action.trim());
    console.log("artifact", artifact.trim());
    console.log("commit", commit.trim());
  }
}

run();
