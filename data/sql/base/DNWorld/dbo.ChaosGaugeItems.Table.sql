USE [dnworld]
GO
/****** Object:  Table [dbo].[ChaosGaugeItems]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ChaosGaugeItems](
	[CharacterID] [bigint] NOT NULL,
	[ChaosGaugeType] [int] NOT NULL,
	[ItemGauge] [int] NOT NULL,
 CONSTRAINT [PK_ChaosGaugeItems] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[ChaosGaugeType] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
