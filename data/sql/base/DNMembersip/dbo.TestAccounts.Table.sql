USE [dnmembership]
GO
/****** Object:  Table [dbo].[TestAccounts]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TestAccounts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[TestAccounts](
	[AccountID] [int] NOT NULL,
	[IP] [int] NOT NULL,
	[AuditLogID] [int] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_TestAccounts] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC,
	[IP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_TestAccounts_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[TestAccounts]'))
ALTER TABLE [dbo].[TestAccounts]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_TestAccounts_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_TestAccounts_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[TestAccounts]'))
ALTER TABLE [dbo].[TestAccounts] CHECK CONSTRAINT [FK_Accounts_TestAccounts_AccountID]
GO
