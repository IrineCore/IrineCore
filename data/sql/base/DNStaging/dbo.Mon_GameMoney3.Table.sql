USE [dnstaging]
GO
/****** Object:  Table [dbo].[Mon_GameMoney3]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Mon_GameMoney3](
	[BasisDate] [date] NOT NULL,
	[CoinChangeCode] [tinyint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[GameMoneyAmount] [bigint] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_Mon_GameMoney3_BasisDate_CoinChangeCode_GameMoneyAmount]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_Mon_GameMoney3_BasisDate_CoinChangeCode_GameMoneyAmount] ON [dbo].[Mon_GameMoney3]
(
	[BasisDate] ASC,
	[CoinChangeCode] ASC,
	[GameMoneyAmount] DESC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
