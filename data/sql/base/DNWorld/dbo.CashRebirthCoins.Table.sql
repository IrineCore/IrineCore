USE [dnworld]
GO
/****** Object:  Table [dbo].[CashRebirthCoins]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CashRebirthCoins](
	[CashRebirthCoinID] [bigint] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CoinCount] [smallint] NOT NULL,
	[RemainCoinCount] [smallint] NOT NULL,
	[CoinPrice] [int] NOT NULL,
	[RemainCoinPrice] [int] NOT NULL,
	[CashRebirthCoinGetCode] [tinyint] NOT NULL,
	[CashRebirthCoinGetKey] [bigint] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[consumptionTargetFlag] [bit] NOT NULL,
 CONSTRAINT [PK_CashRebirthCoins] PRIMARY KEY CLUSTERED 
(
	[CashRebirthCoinID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NNI_CashRebirthCoins_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_CashRebirthCoins_CharacterID] ON [dbo].[CashRebirthCoins]
(
	[CharacterID] ASC
)
INCLUDE([consumptionTargetFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[CashRebirthCoins]  WITH CHECK ADD  CONSTRAINT [CK_CashRebirthCoins_CoinCount] CHECK  (([CoinCount]>(0)))
GO
ALTER TABLE [dbo].[CashRebirthCoins] CHECK CONSTRAINT [CK_CashRebirthCoins_CoinCount]
GO
