import * as core from "@actions/core";
import * as github from "@actions/github";

async function run() {
  const token = core.getInput("token");

  const octokit = new github.GitHub(token);
  const context = github.context;

  await octokit.issues.createComment({
    ...context.repo,
    issue_number: context.payload.pull_request!.number,
    body: `Action: ${core.getInput("action-id")}\n`+
          `Artifact: ${core.getInput("artifact")}\n`+
          `Commit: ${context.sha}`
  });

  // core.setOutput("issue-id", newIssue.data.number.toString());
}

run();
