use GameDB
go

SET NOCOUNT ON
GO

SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

--done: if exists�� ����Ͽ� PlayerTable ���̺��� �����Ѵٸ� �ش� ���̺� ���
IF EXISTS 
	(select * 
	from INFORMATION_SCHEMA.TABLES 
	where TABLE_NAME='PlayerTable') 
DROP TABLE PlayerTable;

CREATE TABLE [dbo].[PlayerTable](
	[playerUID] [int] NOT NULL PRIMARY KEY IDENTITY(100, 1),
	[playerName] [nvarchar](32) NOT NULL DEFAULT (N'noname'),
	[currentPosX] [float] NOT NULL DEFAULT ((0)),
	[currentPosY] [float] NOT NULL DEFAULT ((0)),
	[currentPosZ] [float] NOT NULL DEFAULT ((0)),
	[createTime] [datetime] NOT NULL,
	[isValid] [tinyint] NOT NULL,
	[comment] [nvarchar](256) NULL
)

GO

IF EXISTS ( select * from sys.procedures where name='spCreatePlayer' )
	DROP PROCEDURE [dbo].[spCreatePlayer]
GO

CREATE PROCEDURE [dbo].[spCreatePlayer]
	@name	NVARCHAR(32)
AS
BEGIN
    --done: �ش� �̸��� �÷��̾ �����ϰ� �÷��̾��� identity�� ����, [createTime]�� ���� ���� ��¥�� ����
	SET NOCOUNT ON
	 INSERT INTO [dbo].[PlayerTable] (playerName, createTime, isValid) values (@name, GETDATE(), 1);
	 SELECT @@IDENTITY
END
GO

IF EXISTS ( select * from sys.procedures where name='spDeletePlayer' )
	DROP PROCEDURE [dbo].[spDeletePlayer]
GO

CREATE PROCEDURE [dbo].[spDeletePlayer]
	@playerUID	INT
AS
BEGIN
	--done: �ش� �÷��̾� ����
	SET NOCOUNT ON
	DELETE FROM [dbo].[PlayerTable] where playerUID = @playerUID;
	SELECT @@ROWCOUNT
END
GO

IF EXISTS ( select * from sys.procedures where name='spUpdatePlayerPosition' )
	DROP PROCEDURE [dbo].[spUpdatePlayerPosition]
GO

CREATE PROCEDURE [dbo].[spUpdatePlayerPosition]
	@playerUID	INT,
	@posX	FLOAT,
	@posY	FLOAT,
	@posZ	FLOAT
AS
BEGIN
    -- done: �ش� �÷��̾��� ����(x,y,z) ������Ʈ 
	SET NOCOUNT ON

	UPDATE PlayerTable
	SET currentPosX = @posX,
		currentPosY = @posY,
		currentPosZ = @posZ
	WHERE playerUID = @playerUID;

	SELECT @@ROWCOUNT
END
GO

IF EXISTS ( select * from sys.procedures where name='spUpdatePlayerComment' )
	DROP PROCEDURE [dbo].[spUpdatePlayerComment]
GO

CREATE PROCEDURE [dbo].[spUpdatePlayerComment]
	@playerUID	INT,
	@comment	NVARCHAR(256)
AS
BEGIN
	SET NOCOUNT ON
	UPDATE PlayerTable SET comment=@comment WHERE playerUID=@playerUID
	SELECT @@ROWCOUNT
END
GO

IF EXISTS ( select * from sys.procedures where name='spUpdatePlayerValid' )
	DROP PROCEDURE [dbo].[spUpdatePlayerValid]
GO

CREATE PROCEDURE [dbo].[spUpdatePlayerValid]
	@playerUID	INT,
	@valid		TINYINT
AS
BEGIN
	SET NOCOUNT ON
	UPDATE PlayerTable SET isValid=@valid WHERE playerUID=@playerUID
	SELECT @@ROWCOUNT
END
GO


IF EXISTS ( select * from sys.procedures where name='spLoadPlayer' )
	DROP PROCEDURE [dbo].[spLoadPlayer]
GO

CREATE PROCEDURE [dbo].[spLoadPlayer]
	@playerUID	INT
AS
BEGIN
    --done: �÷��̾� ����  [playerName], [currentPosX], [currentPosY], [currentPosZ], [isValid], [comment]  ������
	SET NOCOUNT ON
	SELECT playerName, currentPosX, currentPosY, currentPosZ, isValid, comment
	from PlayerTable
	where playerUID = @playerUID

END		   
GO		   




--���� ���ν��� �׽�Ʈ

--EXEC spCreatePlayer '�׽�Ʈ�÷��̾�'
--GO

--EXEC spUpdatePlayerComment 100, "�����ٶ� �÷��̾� �ڸ�Ʈ �׽�Ʈ kekeke"
--GO

--EXEC spUpdatePlayerValid 100, 1
--GO

--EXEC spLoadPlayer 100
--GO

--EXEC spDeletePlayer 100
--GO

	
