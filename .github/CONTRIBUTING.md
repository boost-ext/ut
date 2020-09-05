# How to contribute

Contributions are always very much appreciated. However, to make sure the
process of accepting patches goes smoothly for everyone (especially for
the maintainer), you should try to follow these few simple guidelines when
you contribute:

1. Fork the repository.
2. Create a new branch based on the `master` branch (`git checkout -b your_branch master`).
3. Do your modifications on that branch.
4. Be sure your modifications include:
  * Don't break anything (`make`)
  * Proper unit/functional tests (`make test`)
  * Verify performance (`make benchmark`)
5. Commit your changes. Your commit message should reflect your changes.
6. Push the changes to your fork (`git push origin your_branch`).
7. Rebase if necessary to avoid broken commits (`git rebase -i origin/master`).
8. Open a pull request against UT's `master` branch.
   I will do my best to respond in a timely manner.
   I might discuss your patch and suggest some modifications, or I might amend your patch myself and ask you for feedback.
   You will always be given proper credit.
