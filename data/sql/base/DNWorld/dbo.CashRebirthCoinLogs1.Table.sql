USE [dnworld]
GO
/****** Object:  Table [dbo].[CashRebirthCoinLogs1]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CashRebirthCoinLogs1](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CoinCountBefore] [smallint] NOT NULL,
	[CoinCountAfter] [smallint] NOT NULL,
	[MapID] [int] NULL,
	[IP] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_CashRebirthCoinLogs1_LogDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CashRebirthCoinLogs1_LogDate] ON [dbo].[CashRebirthCoinLogs1]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
