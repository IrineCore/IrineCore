USE [dnworld]
GO
/****** Object:  Table [dbo].[CoinLogs2]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CoinLogs2](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CoinTypeCode] [tinyint] NOT NULL,
	[CoinChangeCode] [tinyint] NOT NULL,
	[CurrentCoin] [bigint] NOT NULL,
	[ChangedCoin] [bigint] NOT NULL,
	[ChannelID] [int] NULL,
	[MapID] [int] NULL,
	[CoinChangeFKey] [bigint] NULL,
	[IP] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_CoinLogs2_LogDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CoinLogs2_LogDate] ON [dbo].[CoinLogs2]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CoinLogs2_CharacterID_CoinChangeCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CoinLogs2_CharacterID_CoinChangeCode] ON [dbo].[CoinLogs2]
(
	[CharacterID] ASC,
	[CoinChangeCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
