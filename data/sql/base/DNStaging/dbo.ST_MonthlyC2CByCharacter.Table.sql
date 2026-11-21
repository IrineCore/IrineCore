USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_MonthlyC2CByCharacter]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_MonthlyC2CByCharacter](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[UniqueCharacterCount] [int] NOT NULL,
	[SaleCount] [int] NOT NULL,
	[SaleAmount] [int] NOT NULL,
	[NationalityCode] [tinyint] NULL,
	[UniqueAccountCount] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_MonthlyC2CByCharacter_BasisDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_MonthlyC2CByCharacter_BasisDate] ON [dbo].[ST_MonthlyC2CByCharacter]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
