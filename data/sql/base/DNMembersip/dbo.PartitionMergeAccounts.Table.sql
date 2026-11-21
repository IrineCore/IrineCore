USE [dnmembership]
GO
/****** Object:  Table [dbo].[PartitionMergeAccounts]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PartitionMergeAccounts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PartitionMergeAccounts](
	[AfterAccountID] [int] NOT NULL,
	[BeforeAccountID] [int] NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [PK_PartitionMergeAccounts]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PartitionMergeAccounts]') AND name = N'PK_PartitionMergeAccounts')
ALTER TABLE [dbo].[PartitionMergeAccounts] ADD  CONSTRAINT [PK_PartitionMergeAccounts] PRIMARY KEY NONCLUSTERED 
(
	[AfterAccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
