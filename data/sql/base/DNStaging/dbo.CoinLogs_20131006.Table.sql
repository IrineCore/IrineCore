USE [dnstaging]
GO
/****** Object:  Table [dbo].[CoinLogs_20131006]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CoinLogs_20131006](
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
/****** Object:  Index [IX_NC_CoinLogs_20131006_LogDate]    Script Date: 11/14/2026 9:00:06 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CoinLogs_20131006_LogDate] ON [dbo].[CoinLogs_20131006]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CoinLogs_20131006_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CoinLogs_20131006_CharacterID] ON [dbo].[CoinLogs_20131006]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CoinLogs_20131006_CoinTypeCode_CoinChangeCode_CoinChangeFKey]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CoinLogs_20131006_CoinTypeCode_CoinChangeCode_CoinChangeFKey] ON [dbo].[CoinLogs_20131006]
(
	[CoinTypeCode] ASC,
	[CoinChangeCode] ASC,
	[CoinChangeFKey] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[CoinLogs_20131006]  WITH CHECK ADD  CONSTRAINT [CK_CoinLogs_20131006_LogDate] CHECK  (([LogDate]>='2013-10-06' AND [LogDate]<'2013-10-07'))
GO
ALTER TABLE [dbo].[CoinLogs_20131006] CHECK CONSTRAINT [CK_CoinLogs_20131006_LogDate]
GO
