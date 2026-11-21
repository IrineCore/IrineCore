USE [dnmembership]
GO
/****** Object:  Table [dbo].[TransferAccounts]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TransferAccounts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[TransferAccounts](
	[TransferDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[OldAccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[NewAccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NC_TransferAccounts_TransferDate]    Script Date: 11/14/2026 8:51:04 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[TransferAccounts]') AND name = N'IX_NC_TransferAccounts_TransferDate')
CREATE CLUSTERED INDEX [IX_NC_TransferAccounts_TransferDate] ON [dbo].[TransferAccounts]
(
	[TransferDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
