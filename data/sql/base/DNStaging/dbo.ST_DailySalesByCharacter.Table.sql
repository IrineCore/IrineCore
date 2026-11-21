USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailySalesByCharacter]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailySalesByCharacter](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[PayMethodCode] [tinyint] NOT NULL,
	[PetalIncomeCode] [tinyint] NOT NULL,
	[UniqueCharacterCount] [int] NOT NULL,
	[SaleCount] [int] NOT NULL,
	[SaleAmount] [int] NOT NULL,
	[NationalityCode] [tinyint] NULL,
	[UniqueAccountCount] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailySalesByCharacter_BasisDate_WorldID_JobCode_CharacterLevel_PayMethodCode_PetalIncomeCode_NationalityCode]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailySalesByCharacter_BasisDate_WorldID_JobCode_CharacterLevel_PayMethodCode_PetalIncomeCode_NationalityCode] ON [dbo].[ST_DailySalesByCharacter]
(
	[BasisDate] ASC,
	[WorldID] ASC,
	[JobCode] ASC,
	[CharacterLevel] ASC,
	[PayMethodCode] ASC,
	[PetalIncomeCode] ASC,
	[NationalityCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
