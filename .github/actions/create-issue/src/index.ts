import * as core from "@actions/core";
import * as github from "@actions/github";

async function run() {
  const token = core.getInput("token");

  const octokit = new github.GitHub(token);
  const context = github.context;

  const newIssue = await octokit.issues.create({
    ...context.repo,
    labels: [`buildpack-review`],
    title: `${core.getInput("package-name")} ${core.getInput("package-version")} ready for review`,
    body: `# :rocket: ${core.getInput("package-name")}  ${core.getInput("package-version")} ready for review`
  });

  await octokit.issues.createComment({
    ...context.repo,
    issue_number: newIssue.data.number,
    body: `Action: ${core.getInput("action-id")}`
  });

  await octokit.issues.createComment({
    ...context.repo,
    issue_number: newIssue.data.number,
    body: `Artifact: ${core.getInput("artifact")}`
  });

  await octokit.issues.createComment({
    ...context.repo,
    issue_number: newIssue.data.number,
    body: `Commit: ${context.sha}`
  });

  core.setOutput("issue-id", newIssue.data.number.toString());
}

run();
