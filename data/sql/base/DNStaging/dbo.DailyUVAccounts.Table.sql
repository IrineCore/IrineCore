USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailyUVAccounts]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailyUVAccounts](
	[BasisDate] [date] NOT NULL,
	[AccountID] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_DailyUVAccounts_BasisDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_DailyUVAccounts_BasisDate] ON [dbo].[DailyUVAccounts]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
