USE [dnstaging]
GO
/****** Object:  Table [dbo].[CashRebirthCoinLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CashRebirthCoinLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CoinCountBefore] [smallint] NOT NULL,
	[CoinCountAfter] [smallint] NOT NULL,
	[MapID] [int] NULL,
	[IP] [int] NULL
) ON [PRIMARY]
GO
