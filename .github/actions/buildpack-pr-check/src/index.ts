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

  const actionComment = comments.data.find(
      comment => comment.body.indexOf("Package: ") >= 0 &&
          comment.body.indexOf("Version: ") >= 0 &&
          comment.body.indexOf("Action: ") >= 0 &&
          comment.body.indexOf("Artifact: ") >= 0 &&
          comment.body.indexOf("Commit: ") >= 0
  );

  pack = actionComment.body.substr(comment.body.indexOf("Package: ") + 9)
  pack = pack.subst(pack.indexOf("\n"));
  version = actionComment.body.substr(comment.body.indexOf("Version: ") + 9)
  version = version.subst(version.indexOf("\n"));
  action = actionComment.body.substr(comment.body.indexOf("Action: ") + 8)
  action = action.subst(action.indexOf("\n"));
  artifact = actionComment.body.substr(comment.body.indexOf("Artifact: ") + 10)
  artifact = artifact.subst(artifact.indexOf("\n"));
  commit = actionComment.body.substr(comment.body.indexOf("Commit: ") + 8)

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

run();
