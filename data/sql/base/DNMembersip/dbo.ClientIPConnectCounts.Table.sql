USE [dnmembership]
GO
/****** Object:  Table [dbo].[ClientIPConnectCounts]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[ClientIPConnectCounts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[ClientIPConnectCounts](
	[IP] [int] NOT NULL,
	[AccountID] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL
) ON [DataFG1]
END
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UC_ClientIPConnectCounts_IP_AccountID]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[ClientIPConnectCounts]') AND name = N'IX_UC_ClientIPConnectCounts_IP_AccountID')
CREATE UNIQUE CLUSTERED INDEX [IX_UC_ClientIPConnectCounts_IP_AccountID] ON [dbo].[ClientIPConnectCounts]
(
	[IP] ASC,
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
